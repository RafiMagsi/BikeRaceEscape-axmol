#include "V2/V2Daily.h"
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
int g_liveDist = 0, g_liveCoins = 0, g_liveEvents = 0;

std::string key(int dateKey, const char* suffix) { return "PZP_D" + std::to_string(dateKey) + suffix; }

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
    const std::string s = "dino-bike-escape:v" + std::to_string(kRulesVersion) + ":" + std::to_string(dateKey);
    uint32_t h = 2166136261u;
    for (unsigned char c : s) h = (h ^ c) * 16777619u;
    return h ? h : 1u;
}

Modifier modifierFor(int dateKey) {
    // rotates by day number, so consecutive days always differ
    static const Modifier table[4] = {
        {"standard", "STANDARD RUN", "The same road for everyone today. No revives.", 1.0f, false, 1.0f, 1.0f},
        {"one_life", "ONE LIFE", "No free second chances at all. Score x1.5.", 1.5f, true, 1.0f, 1.0f},
        {"event_rush", "EVENT RUSH", "Road events come twice as often. Score x1.25.", 1.25f, false, 1.0f, 0.5f},
        {"coin_drought", "COIN DROUGHT", "Coins are scarce: earn it with distance and events. Score x1.3.", 1.3f, false, 0.4f, 1.0f},
    };
    const int day = (dateKey / 10000) * 372 + ((dateKey / 100) % 100) * 31 + dateKey % 100;
    return table[day % 4];
}

int computeScore(int distance, int coins, int eventsCleared, const Modifier& m) {
    const float base = static_cast<float>(distance) + 2.f * static_cast<float>(coins) + 25.f * static_cast<float>(eventsCleared);
    return static_cast<int>(base * m.scoreMult);
}

int medalThreshold(int medal) {
    static const int t[4] = {0, 500, 1200, 2500};
    return t[std::max(0, std::min(3, medal))];
}

int medalFor(int score) {
    int m = 0;
    for (int i = 1; i <= 3; ++i) if (score >= medalThreshold(i)) m = i;
    return m;
}

int rewardFor(const Result& r) {
    if (r.mode != Mode::Ranked) return 0;
    static const int byMedal[4] = {20, 40, 80, 150};     // participation counts: a finished ranked run is always worth something
    return byMedal[std::max(0, std::min(3, r.medal))];
}

// ---- persistence ---------------------------------------------------------------------------------------------------------

bool hasRanked(int dateKey) {
    return !UserDefault::getInstance()->getStringForKey(key(dateKey, "_r").c_str(), "").empty();
}

Result rankedResult(int dateKey) {
    Result r;
    r.dateKey = dateKey;
    r.mode = Mode::Ranked;
    const std::string s(UserDefault::getInstance()->getStringForKey(key(dateKey, "_r").c_str(), ""));
    const auto f = split(s, ',');
    if (f.size() >= 6) {
        r.score = std::atoi(f[0].c_str());
        r.distance = std::atoi(f[1].c_str());
        r.coins = std::atoi(f[2].c_str());
        r.eventsCleared = std::atoi(f[3].c_str());
        r.medal = std::atoi(f[4].c_str());
        r.modifierId = f[5];
        r.forfeited = f.size() > 6 && f[6] == "1";
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
    r.medal = medalFor(r.score);
    if (r.mode == Mode::Ranked) {
        if (hasRanked(r.dateKey)) {                       // second ranked write for the same day: ignore
            r.mode = Mode::Practice;
        } else {
            char buf[160];
            std::snprintf(buf, sizeof(buf), "%d,%d,%d,%d,%d,%s,%d", r.score, r.distance, r.coins, r.eventsCleared, r.medal,
                          r.modifierId.c_str(), r.forfeited ? 1 : 0);
            ud->setStringForKey(key(r.dateKey, "_r").c_str(), buf);
            if (!ud->getBoolForKey(key(r.dateKey, "_c").c_str(), false)) {      // reward is claimed once per date
                ud->setBoolForKey(key(r.dateKey, "_c").c_str(), true);
                coinsGranted = rewardFor(r);
            }
            // upload queue: survives offline play, drained by a future leaderboard adapter
            std::string q(ud->getStringForKey("PZP_DQ", ""));
            q += std::to_string(r.dateKey) + "|ranked|" + std::to_string(r.score) + "|" + std::to_string(kRulesVersion) + "|" +
                 r.modifierId + "\n";
            const auto lines = split(q, '\n');
            if (lines.size() > 40) {                          // cap the queue
                std::string trimmed;
                for (size_t i = lines.size() - 40; i < lines.size(); ++i) if (!lines[i].empty()) trimmed += lines[i] + "\n";
                q = trimmed;
            }
            ud->setStringForKey("PZP_DQ", q);
        }
    }
    if (r.mode == Mode::Practice && r.score > bestPractice(r.dateKey)) {
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
void switchToPractice() { if (g_mode == Mode::Ranked) g_mode = Mode::Practice; }
void end() { g_mode = Mode::None; g_active = Modifier(); }

void snapshot(int distance, int coins, int eventsCleared) {
    g_live = true;
    g_liveDist = distance;
    g_liveCoins = coins;
    g_liveEvents = eventsCleared;
}
bool runLive() { return g_live; }
void clearLive() { g_live = false; g_liveDist = g_liveCoins = g_liveEvents = 0; }

bool takeForfeit(Result& out) {
    if (g_mode != Mode::Ranked || !g_live) return false;
    out = Result();
    out.dateKey = dateKeyNow();
    out.mode = Mode::Ranked;
    out.distance = g_liveDist;
    out.coins = g_liveCoins;
    out.eventsCleared = g_liveEvents;
    out.score = computeScore(g_liveDist, g_liveCoins, g_liveEvents, g_active);
    out.modifierId = g_active.id;
    out.forfeited = true;
    clearLive();
    return true;
}

} // namespace Daily
} // namespace V2
