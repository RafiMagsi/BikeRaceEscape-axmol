//
//  PZProgress.h
//  Persistent meta-progression: best score, XP/level, daily reward streak, daily missions.
//  Tunables and text live in PZGameConfig.h. Saved through ax::UserDefault (keys prefixed "PZP_").
//
#pragma once

#include "PZLegacyCompat.h"
#include "PZGameConfig.h"
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

    // --- settings ---
    bool hapticsEnabled() const { return _haptics; }
    void setHapticsEnabled(bool on);

    // --- runs ---
    PZRunResult commitRun(const PZRunStats& stats);

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
    PZMission _missions[PZCfg::kMissionsPerDay];
};
