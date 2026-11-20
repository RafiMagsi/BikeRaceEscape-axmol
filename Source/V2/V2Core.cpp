#include "V2/V2Core.h"
#include "PZLegacyCompat.h"

namespace V2 {

BarkBook::BarkBook() {
    _lines["rival_intro"] = {"Try to keep up. Politely.", "My grandma tunes this bike.", "Last one there is fossil fuel.",
                             "No shortcuts! Unless I find one."};
    _lines["rival_win"]   = {"I meant to let you win.", "Put that on the fridge.", "Okay, that was fast."};
    _lines["rival_loss"]  = {"Rematch? My horn was broken.", "Rawr. See you later!", "Too slow, rookie!"};
    _lines["dino"]        = {"Why is the road so crowded?", "Excuse me! Coming through!", "Dino crossing. Obviously."};
    _lines["cleared"]     = {"Smooth jumping!", "Not a scratch.", "That was legally questionable."};
    _lines["pizza_ok"]    = {"That pizza is still good!", "Delivered. Still warm.", "Five stars!"};
    _lines["pizza_cold"]  = {"The pizza went cold...", "Dropped the order!", "No tip for you."};
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
    if (_sink) _sink(event, std::vector<Field>(fields));
}

} // namespace V2
