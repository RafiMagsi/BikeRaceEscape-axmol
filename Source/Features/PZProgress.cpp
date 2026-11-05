//
//  PZProgress.cpp
//
#include "PZProgress.h"
#include "PZSettingsController.h"

#include <cmath>
#include <ctime>

USING_NS_AX;

namespace {
const char* kKeyBest      = "PZP_Best";
const char* kKeyXp        = "PZP_Xp";
const char* kKeyRuns      = "PZP_Runs";
const char* kKeyLastClaim = "PZP_DailyLast";
const char* kKeyStreak    = "PZP_DailyStreak";
const char* kKeyMissionDay= "PZP_MissionDay";

std::string slotKey(const char* name, int slot) {
    return std::string("PZP_M") + std::to_string(slot) + "_" + name;
}

bool isSingleRunType(PZCfg::MissionType t) {
    using T = PZCfg::MissionType;
    return t == T::RunDistance || t == T::RunCoins || t == T::ReachStage || t == T::ComboChain;
}
} // namespace

PZProgress::PZProgress() { load(); }

PZProgress* PZProgress::shared() {
    static PZProgress* instance = new PZProgress();
    return instance;
}

int PZProgress::todayIndex() {
    std::time_t t = std::time(nullptr);
    long offset = 0;
#if !defined(_WIN32)
    std::tm lt{};
    localtime_r(&t, &lt);
    offset = lt.tm_gmtoff;
#endif
    return static_cast<int>((static_cast<long long>(t) + offset) / 86400);
}

void PZProgress::grantCoins(int amount) {
    if (amount <= 0) return;
    auto* sc = PZSettingsController::shared();
    sc->coins += amount;
    sc->save();
}

// ---- persistence -----------------------------------------------------------------------------

void PZProgress::load() {
    auto* ud = UserDefault::getInstance();
    _best         = ud->getIntegerForKey(kKeyBest, 0);
    _xp           = ud->getIntegerForKey(kKeyXp, 0);
    _totalRuns    = ud->getIntegerForKey(kKeyRuns, 0);
    _lastClaimDay = ud->getIntegerForKey(kKeyLastClaim, -1000);
    _streak       = ud->getIntegerForKey(kKeyStreak, 0);
    _missionDay   = ud->getIntegerForKey(kKeyMissionDay, -1);
    for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
        _missions[i].templateIdx = ud->getIntegerForKey(slotKey("T", i).c_str(), i);
        _missions[i].progress    = ud->getIntegerForKey(slotKey("P", i).c_str(), 0);
        _missions[i].claimed     = ud->getBoolForKey(slotKey("C", i).c_str(), false);
        if (_missions[i].templateIdx < 0 || _missions[i].templateIdx >= PZCfg::kMissionPoolCount) {
            _missions[i].templateIdx = i;
        }
    }
}

void PZProgress::save() {
    auto* ud = UserDefault::getInstance();
    ud->setIntegerForKey(kKeyBest, _best);
    ud->setIntegerForKey(kKeyXp, _xp);
    ud->setIntegerForKey(kKeyRuns, _totalRuns);
    ud->setIntegerForKey(kKeyLastClaim, _lastClaimDay);
    ud->setIntegerForKey(kKeyStreak, _streak);
    ud->setIntegerForKey(kKeyMissionDay, _missionDay);
    for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
        ud->setIntegerForKey(slotKey("T", i).c_str(), _missions[i].templateIdx);
        ud->setIntegerForKey(slotKey("P", i).c_str(), _missions[i].progress);
        ud->setBoolForKey(slotKey("C", i).c_str(), _missions[i].claimed);
    }
    ud->flush();
}

// ---- level -----------------------------------------------------------------------------------

int PZProgress::levelForXp(int xp) {
    return 1 + static_cast<int>(std::floor(std::sqrt(std::max(0, xp) / PZCfg::kXpPerLevelUnit)));
}

float PZProgress::fractionForXp(int xp) {
    const int lv = levelForXp(xp);
    const float lo = PZCfg::kXpPerLevelUnit * (lv - 1) * (lv - 1);
    const float hi = PZCfg::kXpPerLevelUnit * lv * lv;
    return std::max(0.f, std::min(1.f, (xp - lo) / (hi - lo)));
}

// ---- daily reward ----------------------------------------------------------------------------

bool PZProgress::dailyAvailable() const {
    return PZCfg::kEnableDailyReward && todayIndex() != _lastClaimDay;
}

int PZProgress::dailyPendingDay() const {
    const int today = todayIndex();
    const int next = (today == _lastClaimDay + 1) ? _streak + 1 : 1;   // streak breaks after a missed day
    return next;
}

int PZProgress::dailyPendingReward() const {
    const int day = dailyPendingDay();
    return PZCfg::kDailyRewards[(day - 1) % PZCfg::kDailyRewardCount];
}

int PZProgress::claimDaily() {
    if (!dailyAvailable()) return 0;
    const int reward = dailyPendingReward();
    _streak = dailyPendingDay();
    _lastClaimDay = todayIndex();
    grantCoins(reward);
    save();
    return reward;
}

// ---- missions --------------------------------------------------------------------------------

void PZProgress::refreshMissions() {
    const int today = todayIndex();
    if (_missionDay == today) return;

    // Deterministic per-day pick of distinct mission types.
    unsigned seed = static_cast<unsigned>(today) * 2654435761u + 12345u;
    auto next = [&seed]() { seed = seed * 1664525u + 1013904223u; return seed >> 8; };

    int chosen[PZCfg::kMissionsPerDay];
    int n = 0;
    int guard = 0;
    while (n < PZCfg::kMissionsPerDay && guard++ < 200) {
        const int idx = static_cast<int>(next() % PZCfg::kMissionPoolCount);
        bool clash = false;
        for (int i = 0; i < n; ++i) {
            if (chosen[i] == idx || PZCfg::kMissionPool[chosen[i]].type == PZCfg::kMissionPool[idx].type) clash = true;
        }
        if (!clash) chosen[n++] = idx;
    }
    for (; n < PZCfg::kMissionsPerDay; ++n) chosen[n] = n % PZCfg::kMissionPoolCount;

    for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
        _missions[i].templateIdx = chosen[i];
        _missions[i].progress = 0;
        _missions[i].claimed = false;
    }
    _missionDay = today;
    save();
}

const PZCfg::MissionTemplate& PZProgress::missionTemplate(int slot) const {
    return PZCfg::kMissionPool[_missions[slot].templateIdx];
}

bool PZProgress::missionComplete(int slot) const {
    return _missions[slot].progress >= missionTemplate(slot).target;
}

int PZProgress::unclaimedMissions() const {
    if (!PZCfg::kEnableMissions) return 0;
    int n = 0;
    for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
        if (missionComplete(i) && !_missions[i].claimed) ++n;
    }
    return n;
}

int PZProgress::claimMission(int slot) {
    if (slot < 0 || slot >= PZCfg::kMissionsPerDay) return 0;
    if (!missionComplete(slot) || _missions[slot].claimed) return 0;
    _missions[slot].claimed = true;
    const int reward = missionTemplate(slot).reward;
    grantCoins(reward);
    save();
    return reward;
}

std::string PZProgress::missionText(int slot) const {
    const auto& t = missionTemplate(slot);
    char buf[96];
    std::snprintf(buf, sizeof(buf), t.text, t.target);
    return buf;
}

std::string PZProgress::timeUntilNewMissions() const {
    std::time_t t = std::time(nullptr);
    int h = 0, m = 0;
#if !defined(_WIN32)
    std::tm lt{};
    localtime_r(&t, &lt);
    h = 23 - lt.tm_hour;
    m = 59 - lt.tm_min;
#endif
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%dH %02dM", h, m);
    return buf;
}

// ---- run commit ------------------------------------------------------------------------------

PZRunResult PZProgress::commitRun(const PZRunStats& s) {
    refreshMissions();

    PZRunResult r;
    r.prevBest = _best;
    r.levelBefore = level();
    r.fracBefore = levelFraction();

    if (s.distance > _best) {
        r.newBest = s.distance > 0;
        _best = s.distance;
    }
    r.best = _best;
    ++_totalRuns;

    r.xpGained = static_cast<int>(s.distance * PZCfg::kXpPerDistance) + s.coins * PZCfg::kXpPerCoin +
                 (s.stage - 1) * PZCfg::kXpPerStage;
    _xp += r.xpGained;
    r.levelAfter = level();
    r.fracAfter = levelFraction();
    for (int lv = r.levelBefore + 1; lv <= r.levelAfter; ++lv) {
        r.levelUpCoins += PZCfg::kLevelUpCoins * lv;
    }
    grantCoins(r.levelUpCoins);

    if (PZCfg::kEnableMissions) {
        using T = PZCfg::MissionType;
        for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
            PZMission& m = _missions[i];
            if (m.claimed) continue;
            const auto& t = missionTemplate(i);
            const bool wasDone = m.progress >= t.target;

            int single = 0;
            int add = 0;
            switch (t.type) {
            case T::RunDistance: single = s.distance; break;
            case T::RunCoins:    single = s.coins; break;
            case T::ReachStage:  single = s.stage; break;
            case T::ComboChain:  single = s.maxCombo; break;
            case T::TotalCoins:  add = s.coins; break;
            case T::PlayRuns:    add = 1; break;
            case T::NearMisses:  add = s.nearMisses; break;
            case T::PowerUps:    add = s.powerUps; break;
            }
            if (isSingleRunType(t.type)) {
                m.progress = std::max(m.progress, single);
            } else {
                m.progress += add;
            }
            if (!wasDone && m.progress >= t.target) r.completedMissions.push_back(i);
        }
    }

    save();
    return r;
}
