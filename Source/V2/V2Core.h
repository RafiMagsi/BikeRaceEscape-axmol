//
//  V2Core.h
//  Small, dependency-free building blocks for the Version 2 systems: a seeded RNG (reproducible runs), a shuffle bag with
//  cooldowns (no repeats, no streaks), the bark book (short authored lines, never the same one five times in a row) and
//  a platform-neutral analytics port (local ring buffer + log; a real provider plugs in behind it later).
//
#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace V2 {

// xorshift32: tiny, fast, identical on every platform and compiler (std::mt19937 distributions are not portable).
class Rng {
public:
    explicit Rng(uint32_t seed = 1u) { reseed(seed); }
    void reseed(uint32_t seed) { _s0 = seed; _s = seed ? seed : 0x9E3779B9u; for (int i = 0; i < 4; ++i) next(); }
    uint32_t seed() const { return _s0; }
    uint32_t next() {
        uint32_t x = _s;
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        return _s = x;
    }
    float unit() { return static_cast<float>(next() & 0xFFFFFFu) / 16777216.f; }                 // [0,1)
    float range(float a, float b) { return a + (b - a) * unit(); }
    int   index(int n) { return n <= 0 ? 0 : static_cast<int>(next() % static_cast<uint32_t>(n)); }
    bool  chance(float p) { return unit() < p; }
private:
    uint32_t _s = 1, _s0 = 1;
};

// Draws every item once per "bag" in random order; an item is never drawn twice in a row, and an item that was just
// drawn at the end of the previous bag cannot open the next one.
template <class T>
class ShuffleBag {
public:
    void setItems(std::vector<T> items) { _items = std::move(items); _bag.clear(); }
    bool empty() const { return _items.empty(); }
    T next(Rng& rng) {
        if (_bag.empty()) refill(rng);
        T v = _bag.back();
        _bag.pop_back();
        _last = v;
        _hasLast = true;
        return v;
    }
private:
    void refill(Rng& rng) {
        _bag = _items;
        for (int i = static_cast<int>(_bag.size()) - 1; i > 0; --i) std::swap(_bag[i], _bag[rng.index(i + 1)]);
        // _bag.back() is drawn first: keep it different from the last draw
        if (_hasLast && _bag.size() > 1 && _bag.back() == _last) std::swap(_bag.back(), _bag.front());
    }
    std::vector<T> _items, _bag;
    T _last{};
    bool _hasLast = false;
};

// Short authored lines. Max 32 visible characters each (see PRODUCT_PLAN 6.3). Tags: rival_intro, rival_win, rival_loss,
// dino, pizza_ok, pizza_cold, hit_absorbed, cleared.
class BarkBook {
public:
    BarkBook();
    // Returns "" when the tag is unknown. Never repeats any of the last five barks.
    std::string pick(const std::string& tag, Rng& rng);
    void resetHistory() { _recent.clear(); }
private:
    std::map<std::string, std::vector<std::string>> _lines;
    std::deque<std::string> _recent;
};

// Platform-neutral analytics port. Gameplay code calls log(); the sink decides what happens (here: logcat/console and a
// ring buffer a debug screen can read). A real provider (Firebase etc.) is an adapter set with setSink().
struct Field { std::string key, value; };
class Analytics {
public:
    static Analytics& shared();
    void log(const std::string& event, std::initializer_list<Field> fields = {});
    void setSink(std::function<void(const std::string&, const std::vector<Field>&)> sink) { _sink = std::move(sink); }
    const std::deque<std::string>& recent() const { return _recent; }
private:
    std::function<void(const std::string&, const std::vector<Field>&)> _sink;
    std::deque<std::string> _recent;
};

} // namespace V2
