#include "V2/V2Core.h"
#include "PZLegacyCompat.h"
#include "V2/V2Content.h"
#include <cstdio>
#include <ctime>

namespace V2 {

BarkBook::BarkBook() {
    // Authored lines live in CompleteGameInfo.plist (v2.presentation.dialogues); C++ holds no dialogue text.
    Content::shared().load();
    _lines = Content::shared().dialogues();
}

std::string BarkBook::pick(const std::string& tag, Rng& rng) {
    auto it = _lines.find(tag);
    if (it == _lines.end() || it->second.empty()) return "";
    const auto& pool = it->second;
    // up to 8 tries to find a line that is not among the last five; with tiny pools the oldest line is reused
    for (int tries = 0; tries < 8; ++tries) {
        const std::string& cand = pool[rng.index(static_cast<int>(pool.size()))];
        bool seen = false;
        for (const auto& r : _recent) if (r == cand) { seen = true; break; }
        if (!seen || tries == 7) {
            _recent.push_back(cand);
            while (_recent.size() > 5) _recent.pop_front();
            return cand;
        }
    }
    return pool.front();
}

Analytics& Analytics::shared() {
    static Analytics a;
    return a;
}

void Analytics::log(const std::string& event, std::initializer_list<Field> fields) {
    std::string line = event;
    for (const auto& f : fields) line += " " + f.key + "=" + f.value;
    _recent.push_back(line);
    while (_recent.size() > 64) _recent.pop_front();
    AXLOGI("[V2:analytics] {}", line);
    if (!_filePath.empty()) writeLine(event, fields);
    if (_sink) _sink(event, std::vector<Field>(fields));
}

void Analytics::enableFileSink(const std::string& path, size_t maxBytes) {
    if (path == _filePath) return;
    _filePath = path;
    _maxBytes = maxBytes;
}

// One JSON object per line (JSONL): easy to tail, upload or feed to any provider adapter later. The file rotates to
// `<path>.1` when it grows past the limit, so a session can never fill the disk.
void Analytics::writeLine(const std::string& event, const std::vector<Field>& fields) {
    auto esc = [](const std::string& v) {
        std::string o;
        for (char ch : v) {
            if (ch == '"' || ch == '\\') { o += '\\'; o += ch; }
            else if (static_cast<unsigned char>(ch) < 0x20) o += ' ';
            else o += ch;
        }
        return o;
    };
    std::string line = "{\"t\":" + std::to_string(static_cast<long long>(time(nullptr))) + ",\"e\":\"" + esc(event) + "\"";
    for (const auto& f : fields) line += ",\"" + esc(f.key) + "\":\"" + esc(f.value) + "\"";
    line += "}\n";
    FILE* fp = fopen(_filePath.c_str(), "ab");
    if (!fp) return;
    fwrite(line.data(), 1, line.size(), fp);
    const long size = ftell(fp);
    fclose(fp);
    if (size > static_cast<long>(_maxBytes)) {
        const std::string old = _filePath + ".1";
        remove(old.c_str());
        rename(_filePath.c_str(), old.c_str());
    }
}

} // namespace V2
