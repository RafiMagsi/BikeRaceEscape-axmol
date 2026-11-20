#include "V2/V2Daily.h"
#include "V2/V2Config.h"
#include "PZLegacyCompat.h"

#include <algorithm>
#include <ctime>

USING_NS_AX;

namespace V2 {
namespace Daily {

namespace {
int g_dayOffset = 0;
Mode g_mode = Mode::None;
Modifier g_active;
bool g_live = false;
Result g_liveRes;

std::string key(int dateKey, const char* suffix) { return "PZP_D" + std::to_string(dateKey) + suffix; }
bool hasBestStored(int dateKey) { return !UserDefault::getInstance()->getStringForKey(key(dateKey, "_r").c_str(), "").empty(); }

std::vector<std::string> split(const std::string& s, char c) {
    std::vector<std::string> out;
    std::string cur;
    for (char ch : s) {
        if (ch == c) { out.push_back(cur); cur.clear(); }
        else cur.push_back(ch);
    }
    out.push_back(cur);
    return out;
}
}

int debugDayOffset() { return g_dayOffset; }
void setDebugDayOffset(int days) { g_dayOffset = days; }

static std::tm utcTime(int dayOffset) {
    std::time_t t = std::time(nullptr) + static_cast<std::time_t>(dayOffset) * 86400;
    std::tm out{};
    gmtime_r(&t, &out);
    return out;
}

int dateKeyNow() {
    const std::tm t = utcTime(g_dayOffset);
    return (t.tm_year + 1900) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
}

std::string dateLabel(int dateKey) {
    std::tm t{};
    t.tm_year = dateKey / 10000 - 1900;
    t.tm_mon = (dateKey / 100) % 100 - 1;
    t.tm_mday = dateKey % 100;
    t.tm_hour = 12;
    timegm(&t);                                   // fills tm_wday
    static const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    static const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%s %d %s", days[t.tm_wday % 7], t.tm_mday, months[std::max(0, std::min(11, t.tm_mon))]);
    return buf;
}

uint32_t seedFor(int dateKey) {
    // FNV-1a over a fixed string: identical on every platform and compiler.
    const std::string s = "dino-bike-escape:v" + std::to_string(cfg().rulesVersion) + ":" + std::to_string(dateKey);
    uint32_t h = 2166136261u;
    for (unsigned char c : s) h = (h ^ c) * 16777619u;
    return h ? h : 1u;
}

Modifier modifierFor(int dateKey) {
    static const char* titles[6] = {"STANDARD RUN", "ONE LIFE", "EVENT RUSH", "COIN DROUGHT", "PERFECT LANDING DAY", "PIZZA EXPRESS"};
    const Config& c = cfg();
    const int day = (dateKey / 10000) * 372 + ((dateKey / 100) % 100) * 31 + dateKey % 100;     // consecutive days always differ
    const int i = day % 6;
    Modifier m;
    m.index = i;
    m.id = modId(i);
    m.title = titles[i];
    m.scoreMult = c.mods[i].scoreMult;
    m.medalScale = c.mods[i].medalScale;
    m.coinDensityScale = c.mods[i].coins;
    m.eventBreatherScale = c.mods[i].breather;
    m.noMercy = i == 1;
    m.pizzaBoost = i == 5;
    char buf[160];
    switch (i) {
    case 0: m.desc = "The same road for everyone today. No revives."; break;
    case 1: std::snprintf(buf, sizeof(buf), "No free second chances at all. Score x%.2g.", m.scoreMult); m.desc = buf; break;
    case 2: std::snprintf(buf, sizeof(buf), "Road events come twice as often. Score x%.3g.", m.scoreMult); m.desc = buf; break;
    case 3: std::snprintf(buf, sizeof(buf), "Coins are scarce: earn it with distance and events. Score x%.2g.", m.scoreMult); m.desc = buf; break;
    case 4: std::snprintf(buf, sizeof(buf), "Land smooth over gaps: every smooth landing is worth +%d.", static_cast<int>(c.wSmooth)); m.desc = buf; break;
    default: std::snprintf(buf, sizeof(buf), "Pizza deliveries are 3x as common. Box caught +%d, box dropped -%d.", static_cast<int>(c.wBox), static_cast<int>(c.wBoxMiss)); m.desc = buf; break;
    }
    return m;
}

Breakdown breakdown(const Result& r, const Modifier& m) {
    const Config& c = cfg();
    Breakdown b;
    b.distance = static_cast<int>(static_cast<float>(r.distance) * c.wDistance);
    b.coins = static_cast<int>(static_cast<float>(std::max(0, r.coins - r.eventCoins)) * c.wCoins);   // event rewards are not paid twice
    b.events = static_cast<int>(static_cast<float>(r.eventsCleared) * c.wEvents);
    if (m.index == 4) { b.bonus = static_cast<int>(static_cast<float>(r.smooth) * c.wSmooth); b.bonusLabel = "SMOOTH LANDINGS"; }
    if (m.index == 5) {
        b.bonus = static_cast<int>(static_cast<float>(r.boxesCaught) * c.wBox - static_cast<float>(r.boxesMissed) * c.wBoxMiss);
        b.bonusLabel = "PIZZA DELIVERY";
    }
    b.base = std::max(0, b.distance + b.coins + b.events + b.bonus);
    b.mult = m.scoreMult;
    b.total = static_cast<int>(static_cast<float>(b.base) * m.scoreMult);
    return b;
}

int computeScore(const Result& r, const Modifier& m) { return breakdown(r, m).total; }

int medalThreshold(int medal, const Modifier& m) {
    const Config& c = cfg();
    const int t[4] = {0, c.medal1, c.medal2, c.medal3};
    return static_cast<int>(static_cast<float>(t[std::max(0, std::min(3, medal))]) * m.medalScale);
}

int medalFor(int score, const Modifier& m) {
    int k = 0;
    for (int i = 1; i <= 3; ++i) if (score >= medalThreshold(i, m)) k = i;
    return k;
}

// ---- persistence ---------------------------------------------------------------------------------------------------------

int attemptsUsed(int dateKey) { return UserDefault::getInstance()->getIntegerForKey(key(dateKey, "_n").c_str(), 0); }
int attemptsLeft(int dateKey) { return std::max(0, cfg().attemptsPerDay - attemptsUsed(dateKey)); }
bool hasRanked(int dateKey) { return attemptsUsed(dateKey) > 0; }

Result rankedResult(int dateKey) {
    Result r;
    r.dateKey = dateKey;
    r.mode = Mode::Ranked;
    const std::string s(UserDefault::getInstance()->getStringForKey(key(dateKey, "_r").c_str(), ""));
    const auto f = split(s, ',');
    if (f.size() >= 10) {
        r.score = std::atoi(f[0].c_str());
        r.distance = std::atoi(f[1].c_str());
        r.coins = std::atoi(f[2].c_str());
        r.eventsCleared = std::atoi(f[3].c_str());
        r.medal = std::atoi(f[4].c_str());
        r.modifierId = f[5];
        r.forfeited = f[6] == "1";
        r.smooth = std::atoi(f[7].c_str());
        r.boxesCaught = std::atoi(f[8].c_str());
        r.boxesMissed = std::atoi(f[9].c_str());
        if (f.size() > 10) r.eventCoins = std::atoi(f[10].c_str());
    }
    return r;
}

int bestPractice(int dateKey) {
    return UserDefault::getInstance()->getIntegerForKey(key(dateKey, "_p").c_str(), 0);
}

int record(const Result& rIn, int& coinsGranted) {
    Result r = rIn;
    coinsGranted = 0;
    auto* ud = UserDefault::getInstance();
    const Modifier mod = modifierFor(r.dateKey);
    r.score = computeScore(r, mod);
    r.medal = medalFor(r.score, mod);
    if (r.mode == Mode::Ranked && attemptsLeft(r.dateKey) <= 0) r.mode = Mode::Practice;   // attempts used up
    if (r.mode == Mode::Ranked) {
        ud->setIntegerForKey(key(r.dateKey, "_n").c_str(), attemptsUsed(r.dateKey) + 1);
        const bool first = !hasBestStored(r.dateKey);
        if (first || r.score > rankedResult(r.dateKey).score) {
            char buf[200];
            std::snprintf(buf, sizeof(buf), "%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d", r.score, r.distance, r.coins, r.eventsCleared, r.medal,
                          r.modifierId.c_str(), r.forfeited ? 1 : 0, r.smooth, r.boxesCaught, r.boxesMissed, r.eventCoins);
            ud->setStringForKey(key(r.dateKey, "_r").c_str(), buf);
        }
        // reward: paid once per date, then only the difference when the best medal improves
        const int bestMedal = rankedResult(r.dateKey).medal;
        const int target = cfg().rewardByMedal[std::max(0, std::min(3, bestMedal))];
        const int paid = ud->getIntegerForKey(key(r.dateKey, "_c").c_str(), 0);
        if (target > paid) {
            coinsGranted = target - paid;
            ud->setIntegerForKey(key(r.dateKey, "_c").c_str(), target);
        }
        std::string q(ud->getStringForKey("PZP_DQ", ""));
        q += std::to_string(r.dateKey) + "|ranked|" + std::to_string(r.score) + "|" + std::to_string(cfg().rulesVersion) + "|" + r.modifierId + "\n";
        const auto lines = split(q, '\n');
        if (lines.size() > 40) {
            std::string trimmed;
            for (size_t i = lines.size() - 40; i < lines.size(); ++i) if (!lines[i].empty()) trimmed += lines[i] + "\n";
            q = trimmed;
        }
        ud->setStringForKey("PZP_DQ", q);
    } else if (r.score > bestPractice(r.dateKey)) {
        ud->setIntegerForKey(key(r.dateKey, "_p").c_str(), r.score);
    }
    ud->flush();
    return r.medal;
}

std::vector<std::string> pendingUploads() {
    std::vector<std::string> out;
    for (const auto& l : split(std::string(UserDefault::getInstance()->getStringForKey("PZP_DQ", "")), '\n')) {
        if (!l.empty()) out.push_back(l);
    }
    return out;
}

void markUploaded(size_t count) {
    auto lines = pendingUploads();
    std::string q;
    for (size_t i = std::min(count, lines.size()); i < lines.size(); ++i) q += lines[i] + "\n";
    UserDefault::getInstance()->setStringForKey("PZP_DQ", q);
    UserDefault::getInstance()->flush();
}

// ---- session ------------------------------------------------------------------------------------------------------------

void begin(Mode m) {
    g_mode = m;
    g_active = modifierFor(dateKeyNow());
    clearLive();
}

Mode mode() { return g_mode; }
const Modifier& activeModifier() { return g_active; }
void afterRankedResult() {
    if (g_mode == Mode::Ranked && attemptsLeft(dateKeyNow()) <= 0) g_mode = Mode::Practice;
}
void end() { g_mode = Mode::None; g_active = Modifier(); }

void snapshot(const Result& live) { g_live = true; g_liveRes = live; }
bool runLive() { return g_live; }
void clearLive() { g_live = false; g_liveRes = Result(); }

bool takeForfeit(Result& out) {
    if (g_mode != Mode::Ranked || !g_live) return false;
    out = g_liveRes;
    out.dateKey = dateKeyNow();
    out.mode = Mode::Ranked;
    out.modifierId = g_active.id;
    out.forfeited = true;
    clearLive();
    return true;
}

} // namespace Daily
} // namespace V2
