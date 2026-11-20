#include "V2/V2Core.h"
#include "PZLegacyCompat.h"
#include <cstdio>
#include <ctime>

namespace V2 {

BarkBook::BarkBook() {
    _lines["rival_intro"] = {"Try to keep up. Politely.", "My grandma tunes this bike.", "Last one there is fossil fuel.",
                             "No shortcuts! Unless I find one."};
    _lines["rival_grudge"] = {"You again? I want a rematch.", "Ahead on points. Not for long.", "Round two. Bring snacks."};
    _lines["rival_taunt"] = {"Still winning, still smiling.", "Back for more? Love that.", "I have your number now."};
    _lines["trex"]        = {"ROAR! That means hello.", "He only wants a high five.", "Tiny arms, huge feelings."};
    _lines["rival_win"]   = {"I meant to let you win.", "Put that on the fridge.", "Okay, that was fast."};
    _lines["rival_loss"]  = {"Rematch? My horn was broken.", "Rawr. See you later!", "Too slow, rookie!"};
    _lines["dino"]        = {"Why is the road so crowded?", "Excuse me! Coming through!", "Dino crossing. Obviously."};
    _lines["cleared"]     = {"Smooth jumping!", "Not a scratch.", "That was legally questionable."};
    _lines["pizza_ok"]    = {"That pizza is still good!", "Delivered. Still warm.", "Five stars!"};
    _lines["pizza_cold"]  = {"The pizza went cold...", "Dropped the order!", "No tip for you."};
    _lines["construction"] = {"Why are the cones moving?", "Who put a barrel there?", "This is not on the map."};
    _lines["meteor"]      = {"Meteor insurance was extra.", "Look up! Or don't.", "That is not weather."};
    _lines["grandma"]     = {"Tune-up complete, dear!", "Hold my basket!", "Beep beep, sweetie!"};
    _lines["hit_absorbed"] = {"Meteor insurance was extra.", "The road attacked first.", "Bonk! Nobody saw that."};
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
