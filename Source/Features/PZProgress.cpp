//
//  PZProgress.cpp
//
#include "PZProgress.h"
#include <memory>
#include "PZSettingsController.h"

#include <algorithm>
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
const char* kKeyHaptics   = "PZP_Haptics";
const char* kKeyNotifAsks = "PZP_NotifAsks";
const char* kKeyNotifLast = "PZP_NotifLastAsk";
const char* kKeyNotifOn   = "PZP_NotifOn";
const char* kKeyGadget    = "PZP_Gadget";
const char* kKeyGadgetInit= "PZP_GadgetInit";
const char* kKeyFreeCoins = "PZP_FreeCoinsAt";
const char* kKeyHourly    = "PZP_HourlyAt";
const char* kKeySpinDay   = "PZP_SpinDay";
const char* kKeyChest     = "PZP_ChestLevel";
const char* kKeyMChest    = "PZP_MissionChestDay";
const char* kKeyPlayed    = "PZP_LastPlayedAt";
const char* kKeyComeback  = "PZP_ComebackCoins";
const char* kKeyComebackD = "PZP_ComebackDays";

std::string slotKey(const char* name, int slot) {
    return std::string("PZP_M") + std::to_string(slot) + "_" + name;
}

bool isSingleRunType(PZCfg::MissionType t) {
    using T = PZCfg::MissionType;
    return t == T::RunDistance || t == T::RunCoins || t == T::ReachStage || t == T::ComboChain;
}
} // namespace

PZProgress::PZProgress() {
    PZCfg::ensureLoaded();
    load();

    // First install: hand the player the first unlocked gadget so the loadout isn't an empty mystery.
    auto* ud = UserDefault::getInstance();
    if (!ud->getBoolForKey(kKeyGadgetInit, false)) {
        _gadgets.clear();
        for (int i = 0; i < gadgetPoolCount() && static_cast<int>(_gadgets.size()) < PZCfg::kGadgetSlots; ++i) {
            if (gadgetUnlocked(i)) {
                _gadgets.push_back(PZCfg::kGadgetPool[i].id);
                break;
            }
        }
        ud->setBoolForKey(kKeyGadgetInit, true);
        save();
    }
}

int PZProgress::today() { return todayIndex(); }

int PZProgress::daysSinceNotifAsk() const { return todayIndex() - _notifLastAsk; }

void PZProgress::markNotifAsked() {
    ++_notifAsks;
    _notifLastAsk = todayIndex();
    auto* ud = UserDefault::getInstance();
    ud->setIntegerForKey(kKeyNotifAsks, _notifAsks);
    ud->setIntegerForKey(kKeyNotifLast, _notifLastAsk);
    ud->flush();
}

void PZProgress::setNotifEnabled(bool on) {
    _notifOn = on;
    UserDefault::getInstance()->setBoolForKey(kKeyNotifOn, on);
    UserDefault::getInstance()->flush();
}

void PZProgress::setHapticsEnabled(bool on) {
    _haptics = on;
    UserDefault::getInstance()->setBoolForKey(kKeyHaptics, on);
    UserDefault::getInstance()->flush();
}

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
    _haptics      = ud->getBoolForKey(kKeyHaptics, PZCfg::kEnableHaptics);
    _notifAsks    = ud->getIntegerForKey(kKeyNotifAsks, 0);
    _notifLastAsk = ud->getIntegerForKey(kKeyNotifLast, -1000);
    _notifOn      = ud->getBoolForKey(kKeyNotifOn, true);
    _lastFreeCoins = ud->getIntegerForKey(kKeyFreeCoins, -100000);
    _lastHourly   = ud->getIntegerForKey(kKeyHourly, -100000);
    _spinDay      = ud->getIntegerForKey(kKeySpinDay, -1000);
    _chestClaimedLevel = ud->getIntegerForKey(kKeyChest, 0);
    _missionChestDay = ud->getIntegerForKey(kKeyMChest, -1000);
    _lastPlayed   = ud->getIntegerForKey(kKeyPlayed, 0);
    _comebackCoins = ud->getIntegerForKey(kKeyComeback, 0);
    _comebackDays  = ud->getIntegerForKey(kKeyComebackD, 0);

    // Welcome-back gift: measured against the last session, then the clock is restarted. An unclaimed gift persists.
    const int nowSec = static_cast<int>(std::time(nullptr));
    if (_lastPlayed > 0 && _comebackCoins <= 0) {
        const int away = nowSec - _lastPlayed;
        if (PZCfg::kEnableGifts && away >= PZCfg::kComebackMinHours * 3600) {
            _comebackDays = std::max(1, std::min(PZCfg::kComebackMaxDays, away / 86400));
            _comebackCoins = _comebackDays * PZCfg::kComebackCoinsPerDay;
            ud->setIntegerForKey(kKeyComeback, _comebackCoins);
            ud->setIntegerForKey(kKeyComebackD, _comebackDays);
        }
    }
    _lastPlayed = nowSec;
    ud->setIntegerForKey(kKeyPlayed, _lastPlayed);

    _gadgets.clear();
    const int slots = std::max(0, PZCfg::kGadgetSlots);
    for (int i = 0; i < slots; ++i) {
        const std::string key = std::string(kKeyGadget) + std::to_string(i);
        const auto sv = ud->getStringForKey(key.c_str(), "");
        const std::string id(sv.data(), sv.size());
        const int idx = gadgetPoolIndex(id);
        if (idx >= 0 && gadgetUnlocked(idx) && !gadgetSelected(id)) _gadgets.push_back(id);
    }
    _stock.clear();
    for (const auto& g : PZCfg::kGadgetPool) {
        _stock[g.id] = std::max(0, ud->getIntegerForKey((std::string("PZP_GStock_") + g.id).c_str(), std::max(0, PZCfg::kGadgetStartStock)));
    }
    for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
        _missions[i].templateIdx = ud->getIntegerForKey(slotKey("T", i).c_str(), i);
        _missions[i].progress    = ud->getIntegerForKey(slotKey("P", i).c_str(), 0);
        _missions[i].claimed     = ud->getBoolForKey(slotKey("C", i).c_str(), false);
        if (_missions[i].templateIdx < 0 || _missions[i].templateIdx >= static_cast<int>(PZCfg::kMissionPool.size())) {
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
    ud->setIntegerForKey(kKeyFreeCoins, _lastFreeCoins);
    for (int i = 0; i < std::max(0, PZCfg::kGadgetSlots); ++i) {
        const std::string key = std::string(kKeyGadget) + std::to_string(i);
        ud->setStringForKey(key.c_str(), i < static_cast<int>(_gadgets.size()) ? _gadgets[i] : std::string());
    }
    for (const auto& kv : _stock) ud->setIntegerForKey((std::string("PZP_GStock_") + kv.first).c_str(), kv.second);
    for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
        ud->setIntegerForKey(slotKey("T", i).c_str(), _missions[i].templateIdx);
        ud->setIntegerForKey(slotKey("P", i).c_str(), _missions[i].progress);
        ud->setBoolForKey(slotKey("C", i).c_str(), _missions[i].claimed);
    }
    ud->flush();
}

// ---- gadgets ---------------------------------------------------------------------------------

int PZProgress::gadgetPoolIndex(const std::string& id) const {
    for (int i = 0; i < gadgetPoolCount(); ++i) {
        if (PZCfg::kGadgetPool[i].id == id) return i;
    }
    return -1;
}

const std::string& PZProgress::gadgetAt(int slot) const {
    static const std::string empty;
    if (slot < 0 || slot >= static_cast<int>(_gadgets.size())) return empty;
    return _gadgets[slot];
}

bool PZProgress::gadgetUnlocked(int poolIndex) const {
    if (poolIndex < 0 || poolIndex >= gadgetPoolCount()) return false;
    return level() >= PZCfg::kGadgetPool[poolIndex].unlockLevel;
}

bool PZProgress::gadgetSelected(const std::string& id) const {
    return std::find(_gadgets.begin(), _gadgets.end(), id) != _gadgets.end();
}

int PZProgress::selectedGadgetCount() const {
    return static_cast<int>(_gadgets.size());
}

void PZProgress::toggleGadget(const std::string& id) {
    const int idx = gadgetPoolIndex(id);
    if (idx < 0 || !gadgetUnlocked(idx)) return;
    auto it = std::find(_gadgets.begin(), _gadgets.end(), id);
    if (it != _gadgets.end()) {
        _gadgets.erase(it);
    } else if (static_cast<int>(_gadgets.size()) < std::max(0, PZCfg::kGadgetSlots)) {
        _gadgets.push_back(id);
    } else {
        return;   // loadout full: deselect a card first
    }
    save();
}

int PZProgress::gadgetStock(const std::string& id) const {
    auto it = _stock.find(id);
    return it == _stock.end() ? 0 : it->second;
}

void PZProgress::addGadgetStock(const std::string& id, int n) {
    if (gadgetPoolIndex(id) < 0) return;
    _stock[id] = std::max(0, gadgetStock(id) + n);
    save();
}

bool PZProgress::consumeGadgetStock(const std::string& id) {
    if (gadgetStock(id) <= 0) return false;
    _stock[id] = gadgetStock(id) - 1;
    save();
    return true;
}

bool PZProgress::buyGadget(const std::string& id) {
    const int idx = gadgetPoolIndex(id);
    if (idx < 0 || !gadgetUnlocked(idx)) return false;
    auto* sc = PZSettingsController::shared();
    const int price = PZCfg::kGadgetPool[idx].price;
    if (sc->coins < price) return false;
    sc->coins = static_cast<int>(sc->coins) - price;
    sc->save();
    addGadgetStock(id, 1);
    return true;
}

// ---- rewarded-video economy ------------------------------------------------------------------

int PZProgress::freeCoinsSecondsLeft() const {
    const int now = static_cast<int>(std::time(nullptr));
    const int elapsed = now - _lastFreeCoins;
    return std::max(0, PZCfg::kFreeCoinsCooldown - elapsed);
}

bool PZProgress::freeCoinsAvailable() const {
    return PZCfg::kEnableFreeCoins && freeCoinsSecondsLeft() <= 0;
}

// ---- gifts -----------------------------------------------------------------------------------

int PZProgress::hourlySecondsLeft() const {
    const int now = static_cast<int>(std::time(nullptr));
    return std::max(0, PZCfg::kHourlyCooldown - (now - _lastHourly));
}

bool PZProgress::hourlyAvailable() const { return PZCfg::kEnableGifts && hourlySecondsLeft() <= 0; }

int PZProgress::claimHourly() {
    if (!hourlyAvailable()) return 0;
    const int lo = std::min(PZCfg::kHourlyMin, PZCfg::kHourlyMax), hi = std::max(PZCfg::kHourlyMin, PZCfg::kHourlyMax);
    const int amount = lo + static_cast<int>(CCRANDOM_0_1() * (hi - lo + 1));
    _lastHourly = static_cast<int>(std::time(nullptr));
    awardCoins(amount);
    auto* ud = UserDefault::getInstance();
    ud->setIntegerForKey(kKeyHourly, _lastHourly);
    ud->flush();
    return amount;
}

bool PZProgress::spinAvailable() const { return PZCfg::kEnableGifts && _spinDay != todayIndex(); }

int PZProgress::pickSpinIndex() const {
    int total = 0;
    for (int w : PZCfg::kSpinWeights) total += std::max(0, w);
    if (total <= 0) return 0;
    int r = static_cast<int>(CCRANDOM_0_1() * total);
    for (size_t i = 0; i < PZCfg::kSpinWeights.size(); ++i) {
        r -= std::max(0, PZCfg::kSpinWeights[i]);
        if (r < 0) return static_cast<int>(i);
    }
    return 0;
}

int PZProgress::claimSpin(int index, bool freeSpin) {
    if (index < 0 || index >= static_cast<int>(PZCfg::kSpinRewards.size())) return 0;
    const int amount = PZCfg::kSpinRewards[index];
    awardCoins(amount);
    if (freeSpin) {
        _spinDay = todayIndex();
        UserDefault::getInstance()->setIntegerForKey(kKeySpinDay, _spinDay);
        UserDefault::getInstance()->flush();
    }
    return amount;
}

int PZProgress::chestLevelPending() const {
    if (!PZCfg::kEnableGifts || PZCfg::kChestEveryLevels <= 0) return 0;
    const int every = PZCfg::kChestEveryLevels;
    const int reached = (level() / every) * every;       // highest chest level the player has reached
    return reached > _chestClaimedLevel && reached >= every ? reached : 0;
}

int PZProgress::nextChestLevel() const {
    const int every = std::max(1, PZCfg::kChestEveryLevels);
    return (std::max(level(), _chestClaimedLevel) / every + 1) * every;
}

int PZProgress::claimChest() {
    const int lv = chestLevelPending();
    if (lv <= 0) return 0;
    const int amount = lv * PZCfg::kChestCoinsPerLevel;
    _chestClaimedLevel = lv;
    awardCoins(amount);
    UserDefault::getInstance()->setIntegerForKey(kKeyChest, _chestClaimedLevel);
    UserDefault::getInstance()->flush();
    return amount;
}

bool PZProgress::missionChestAvailable() const {
    if (!PZCfg::kEnableGifts || !PZCfg::kEnableMissions || _missionChestDay == todayIndex()) return false;
    for (int i = 0; i < PZCfg::kMissionsPerDay; ++i) {
        if (!_missions[i].claimed) return false;
    }
    return true;
}

int PZProgress::claimMissionChest() {
    if (!missionChestAvailable()) return 0;
    _missionChestDay = todayIndex();
    awardCoins(PZCfg::kMissionChestCoins);
    UserDefault::getInstance()->setIntegerForKey(kKeyMChest, _missionChestDay);
    UserDefault::getInstance()->flush();
    return PZCfg::kMissionChestCoins;
}

int PZProgress::claimComeback() {
    if (_comebackCoins <= 0) return 0;
    const int amount = _comebackCoins;
    _comebackCoins = 0;
    _comebackDays = 0;
    awardCoins(amount);
    auto* ud = UserDefault::getInstance();
    ud->setIntegerForKey(kKeyComeback, 0);
    ud->setIntegerForKey(kKeyComebackD, 0);
    ud->flush();
    return amount;
}

int PZProgress::giftsReady() const {
    int n = 0;
    if (dailyAvailable()) ++n;
    if (hourlyAvailable()) ++n;
    if (spinAvailable()) ++n;
    if (chestLevelPending() > 0) ++n;
    if (missionChestAvailable()) ++n;
    if (_comebackCoins > 0) ++n;
    return n;
}

int PZProgress::claimFreeCoins(int amount) {
    if (!freeCoinsAvailable() || amount <= 0) return 0;
    _lastFreeCoins = static_cast<int>(std::time(nullptr));
    awardCoins(amount);
    save();
    return amount;
}

void PZProgress::awardCoins(int amount) {
    grantCoins(amount);
}

// ---- level -----------------------------------------------------------------------------------

int PZProgress::levelForXp(int xp) {
    return 1 + static_cast<int>(std::floor(std::sqrt(std::max(0, xp) / PZCfg::kXpPerLevelUnit)));
}

int PZProgress::xpIntoLevel() const {
    const int lv = level();
    return std::max(0, _xp - static_cast<int>(PZCfg::kXpPerLevelUnit * (lv - 1) * (lv - 1)));
}

int PZProgress::xpSpanOfLevel() const {
    const int lv = level();
    return std::max(1, static_cast<int>(PZCfg::kXpPerLevelUnit * lv * lv) - static_cast<int>(PZCfg::kXpPerLevelUnit * (lv - 1) * (lv - 1)));
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
    return PZCfg::kDailyRewards[(day - 1) % static_cast<int>(PZCfg::kDailyRewards.size())];
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
        const int idx = static_cast<int>(next() % PZCfg::kMissionPool.size());
        bool clash = false;
        for (int i = 0; i < n; ++i) {
            if (chosen[i] == idx || PZCfg::kMissionPool[chosen[i]].type == PZCfg::kMissionPool[idx].type) clash = true;
        }
        if (!clash) chosen[n++] = idx;
    }
    for (; n < PZCfg::kMissionsPerDay; ++n) chosen[n] = n % static_cast<int>(PZCfg::kMissionPool.size());

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

int PZProgress::livePreview(int slot, const PZRunStats& s) const {
    using T = PZCfg::MissionType;
    const PZMission& m = _missions[slot];
    const auto& t = missionTemplate(slot);
    int cur = 0;
    switch (t.type) {
    case T::RunDistance: cur = s.distance; break;
    case T::RunCoins:    cur = s.coins; break;
    case T::ReachStage:  cur = s.stage; break;
    case T::ComboChain:  cur = s.maxCombo; break;
    case T::TotalCoins:  cur = s.coins; break;
    case T::PlayRuns:    cur = 0; break;
    case T::NearMisses:  cur = s.nearMisses; break;
    case T::PowerUps:    cur = s.powerUps; break;
    case T::SmashObstacles: cur = s.smashed; break;
    }
    return isSingleRunType(t.type) ? std::max(m.progress, cur) : m.progress + cur;
}

std::string PZProgress::missionText(int slot) const {
    const auto& t = missionTemplate(slot);
    char buf[96];
    std::snprintf(buf, sizeof(buf), t.text.c_str(), t.target);
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

static std::shared_ptr<PZProgress>& progressSnapshot() {
    static std::shared_ptr<PZProgress> snap;
    return snap;
}

void PZProgress::snapshot() { progressSnapshot() = std::make_shared<PZProgress>(*this); }

bool PZProgress::restoreSnapshot() {
    auto& snap = progressSnapshot();
    if (!snap) return false;
    *this = *snap;
    snap.reset();
    save();
    return true;
}

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
            case T::SmashObstacles: add = s.smashed; break;
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
