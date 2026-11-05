//
//  PZProgress.h
//  Persistent meta-progression: best score, XP/level, daily reward streak, daily missions.
//  Tunables and text live in PZGameConfig.h. Saved through ax::UserDefault (keys prefixed "PZP_").
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZGameConfig.h"
#include <map>
#include <string>
#include <vector>

// Raw numbers gathered during one run (filled by PZRunDirector).
struct PZRunStats {
    int  distance   = 0;
    int  coins      = 0;   // coins picked up this run (incl. bonuses)
    int  maxCombo   = 0;
    int  nearMisses = 0;
    int  powerUps   = 0;
    int  smashed    = 0;   // obstacles destroyed (shield / turbo / blast)
    int  stage      = 1;
    bool revived    = false;
};

// What changed when a run was committed (drives the game-over panel).
struct PZRunResult {
    bool newBest        = false;
    int  prevBest       = 0;
    int  best           = 0;
    int  xpGained       = 0;
    int  levelBefore    = 1;
    int  levelAfter     = 1;
    float fracBefore    = 0.f;   // progress inside the level, 0..1
    float fracAfter     = 0.f;
    int  levelUpCoins   = 0;
    std::vector<int> completedMissions;   // slots that reached their target in this run
};

struct PZMission {
    int  templateIdx = 0;
    int  progress    = 0;
    bool claimed     = false;
};

class PZProgress {
public:
    static PZProgress* shared();

    // --- score / level ---
    int   bestScore() const { return _best; }
    int   xp() const { return _xp; }
    int   level() const { return levelForXp(_xp); }
    float levelFraction() const { return fractionForXp(_xp); }
    int   xpIntoLevel() const;                  // XP earned inside the current level
    int   xpSpanOfLevel() const;                // XP needed to clear the current level
    static int   levelForXp(int xp);
    static float fractionForXp(int xp);

    // --- daily reward ---
    bool dailyAvailable() const;
    int  dailyPendingDay() const;      // 1-based day (within the reward table) that Claim would pay
    int  dailyPendingReward() const;
    int  streak() const { return _streak; }
    int  claimDaily();                 // returns coins granted (0 if not available)

    // --- missions ---
    void refreshMissions();            // rolls a new set when the day changes
    int  missionCount() const { return PZCfg::kMissionsPerDay; }
    const PZMission& mission(int slot) const { return _missions[slot]; }
    const PZCfg::MissionTemplate& missionTemplate(int slot) const;
    bool missionComplete(int slot) const;
    int  unclaimedMissions() const;
    int  claimMission(int slot);       // returns coins granted
    // Progress of a mission including the run in flight (for the in-run tracker).
    int  livePreview(int slot, const PZRunStats& stats) const;
    std::string missionText(int slot) const;
    std::string timeUntilNewMissions() const;

    // --- gadgets (menu loadout; ids come from PZCfg::kGadgetPool) ---
    int  gadgetSlotCount() const { return PZCfg::kGadgetSlots; }
    int  gadgetPoolCount() const { return static_cast<int>(PZCfg::kGadgetPool.size()); }
    int  gadgetPoolIndex(const std::string& id) const;
    const std::string& gadgetAt(int slot) const;
    bool gadgetUnlocked(int poolIndex) const;
    bool gadgetSelected(const std::string& id) const;
    int  selectedGadgetCount() const;
    void toggleGadget(const std::string& id);
    // Charges: buying adds one, every equipped gadget uses one at the start of a run.
    int  gadgetStock(const std::string& id) const;
    void addGadgetStock(const std::string& id, int n);
    bool consumeGadgetStock(const std::string& id);
    bool buyGadget(const std::string& id);      // spends coins (false: locked or not enough coins)

    // --- rewarded-video economy ---
    bool freeCoinsAvailable() const;
    int  freeCoinsSecondsLeft() const;      // 0 when available
    int  claimFreeCoins(int amount);        // grants the coins and starts the cooldown
    void awardCoins(int amount);            // coins from a rewarded video / bonus, saved immediately

    // --- gifts ---
    bool hourlyAvailable() const;
    int  hourlySecondsLeft() const;
    int  claimHourly();                     // random coins in [HourlyMin, HourlyMax]; starts the cooldown
    bool spinAvailable() const;             // one free spin per day
    int  pickSpinIndex() const;             // weighted random wheel segment
    int  claimSpin(int index, bool freeSpin);   // grants the segment's coins; a free spin uses up the day's spin
    int  chestLevelPending() const;         // highest unclaimed chest level reached (0 = none)
    int  nextChestLevel() const;
    int  claimChest();
    bool missionChestAvailable() const;     // all of today's missions claimed, chest not yet taken
    int  claimMissionChest();
    int  comebackCoins() const { return _comebackCoins; }   // > 0 when a welcome-back gift is waiting
    int  comebackDays() const { return _comebackDays; }
    int  claimComeback();
    int  giftsReady() const;                // badge count for the GIFTS button

    // --- reminders (local notifications) ---
    int  totalRuns() const { return _totalRuns; }
    int  notifAsks() const { return _notifAsks; }
    int  daysSinceNotifAsk() const;               // large when never asked
    void markNotifAsked();
    bool notifEnabled() const { return _notifOn; }
    void setNotifEnabled(bool on);
    static int today();                           // local day index

    // --- settings ---
    bool hapticsEnabled() const { return _haptics; }
    void setHapticsEnabled(bool on);

    // --- runs ---
    PZRunResult commitRun(const PZRunStats& stats);
    // Game-over revive: remember the state before a run is committed and roll back to it if the player revives.
    // (Coins are not part of the snapshot; the caller removes the level-up coins that commitRun granted.)
    void snapshot();
    bool restoreSnapshot();

    void save();

private:
    PZProgress();
    void load();
    static int todayIndex();
    static void grantCoins(int amount);

    int  _best = 0;
    int  _xp = 0;
    int  _totalRuns = 0;
    int  _lastClaimDay = -1000;
    int  _streak = 0;
    int  _missionDay = -1;
    bool _haptics = true;
    int  _notifAsks = 0;
    int  _notifLastAsk = -1000;
    bool _notifOn = true;
    std::vector<std::string> _gadgets;      // equipped gadget ids (validated on load)
    std::map<std::string, int> _stock;      // gadget id -> charges owned
    int  _lastFreeCoins = -100000;
    int  _lastHourly = -100000;
    int  _spinDay = -1000;
    int  _chestClaimedLevel = 0;
    int  _missionChestDay = -1000;
    int  _lastPlayed = 0;
    int  _comebackCoins = 0;
    int  _comebackDays = 0;
    PZMission _missions[PZCfg::kMissionsPerDay];
};
