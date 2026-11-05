//
//  PZGameConfig.h
//  Single place to retheme / retune the "meta" features (progression, power-ups, missions, popups).
//
//  RESKIN GUIDE
//  ------------
//  Every colour, string, number and optional art override used by the code-drawn features lives here.
//  A reskin only has to edit this file (plus the usual atlas/plist art): none of the feature UI uses
//  sprite sheets. Optional art overrides (see kArt*) are picked up automatically when the file exists
//  in Content/Images; otherwise a code-drawn fallback is used.
//
#pragma once

#include "axmol.h"

namespace PZCfg {

// ---------------------------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------------------------
inline const char* kFontPath = "Fonts/PressStart2P-Regular.ttf";

inline const ax::Color3B kColText      {255, 255, 255};
inline const ax::Color3B kColDim       {150, 165, 185};
inline const ax::Color3B kColAccent    {255, 205,  40};   // gold: coins, headings
inline const ax::Color3B kColAccent2   { 40, 220, 255};   // cyan: highlights, level bar
inline const ax::Color3B kColGood      { 90, 235, 120};   // positive / claim
inline const ax::Color3B kColBad       {255,  80,  80};   // danger / countdown low
inline const ax::Color3B kColPanel     { 14,  22,  38};   // panel fill
inline const ax::Color3B kColPanelEdge { 70, 200, 255};   // panel border

// Power-up identity colours (also used for HUD bars, orbs, auras).
inline const ax::Color3B kColMagnet    {255,  90, 200};
inline const ax::Color3B kColShield    { 70, 170, 255};
inline const ax::Color3B kColTurbo     {255, 140,  30};

// Per-stage atmosphere tint (cycled). RGB + alpha (0-255) of a full-screen overlay.
struct StageTint { unsigned char r, g, b, a; };
inline constexpr StageTint kStageTints[] = {
    {  0,   0,   0,   0},   // stage 1: untouched art
    {255, 120,  40,  38},   // stage 2: dusk
    { 40,  60, 200,  58},   // stage 3: night
    { 60, 255, 120,  44},   // stage 4: toxic
    {200,  40, 255,  52},   // stage 5: neon
};
inline constexpr int kStageTintCount = sizeof(kStageTints) / sizeof(kStageTints[0]);

// ---------------------------------------------------------------------------------------------
// Text (edit for the new theme / localisation). printf-style where a %d appears.
// ---------------------------------------------------------------------------------------------
inline const char* kStrBest          = "BEST %d";
inline const char* kStrNewBest       = "NEW BEST!";
inline const char* kStrStage         = "STAGE %d";
inline const char* kStrStageSub      = "+%d COINS";
inline const char* kStrCloseCall     = "CLOSE CALL! +%d";
inline const char* kStrCombo         = "COMBO x%d";
inline const char* kStrComboBonus    = "COMBO BONUS +%d";
inline const char* kStrSmash         = "SMASH!";
inline const char* kStrShieldBreak   = "SHIELD!";

inline const char* kStrMagnet        = "MAGNET";
inline const char* kStrShield        = "SHIELD";
inline const char* kStrTurbo         = "TURBO";
inline const char* kStrMagnetIcon    = "M";
inline const char* kStrShieldIcon    = "S";
inline const char* kStrTurboIcon     = "T";

inline const char* kStrReviveTitle   = "CONTINUE?";
inline const char* kStrReviveBtn     = "REVIVE  %d";
inline const char* kStrReviveNoCoins = "NEED %d COINS";
inline const char* kStrReviveSkip    = "NO THANKS";
inline const char* kStrReviveCoins   = "YOU HAVE %d";

inline const char* kStrDailyTitle    = "DAILY REWARD";
inline const char* kStrDailyDay      = "DAY %d";
inline const char* kStrDailyClaim    = "CLAIM";
inline const char* kStrDailyStreak   = "%d DAY STREAK";
inline const char* kStrDailyBtn      = "DAILY";

inline const char* kStrMissionsTitle = "DAILY MISSIONS";
inline const char* kStrMissionsBtn   = "MISSIONS";
inline const char* kStrMissionClaim  = "CLAIM";
inline const char* kStrMissionDone   = "DONE";
inline const char* kStrMissionNew    = "NEW MISSIONS IN %s";
inline const char* kStrClose         = "CLOSE";

inline const char* kStrResultsBest   = "BEST %d";
inline const char* kStrResultsCoins  = "COINS +%d";
inline const char* kStrResultsLevel  = "LEVEL %d";
inline const char* kStrLevelUp       = "LEVEL UP!  +%d";
inline const char* kStrMissionToast  = "MISSION COMPLETE!";
inline const char* kStrLevelPlate    = "LV %d";

// ---------------------------------------------------------------------------------------------
// Feature switches
// ---------------------------------------------------------------------------------------------
inline constexpr bool kEnablePowerUps    = true;
inline constexpr bool kEnableRevive      = true;
inline constexpr bool kEnableStages      = true;
inline constexpr bool kEnableCombos      = true;
inline constexpr bool kEnableNearMiss    = true;
inline constexpr bool kEnableDailyReward = true;
inline constexpr bool kEnableMissions    = true;
inline constexpr bool kEnableShake       = true;

// ---------------------------------------------------------------------------------------------
// Tuning
// ---------------------------------------------------------------------------------------------
// Stages: distance (score units) per stage, and coin bonus granted on entering a stage.
inline constexpr float kStageLength      = 350.0f;
inline constexpr int   kStageCoinBonus   = 10;

// Combo: consecutive coin pickups within this window build a combo; every N-chain pays a bonus.
inline constexpr float kComboWindow      = 1.6f;
inline constexpr int   kComboBonusEvery  = 5;
inline constexpr int   kComboBonusCoins  = 3;

// Near miss: obstacle passing within this vertical distance (design units) of the rider while not hit.
inline constexpr float kNearMissRange    = 70.0f;
inline constexpr int   kNearMissCoins    = 2;

// Power-ups.
inline constexpr float kPowerUpFirstDelay   = 9.0f;    // seconds of run time before the first orb
inline constexpr float kPowerUpIntervalMin  = 11.0f;
inline constexpr float kPowerUpIntervalMax  = 20.0f;
inline constexpr float kOrbHeightAboveGround= 120.0f;  // reachable with a jump
inline constexpr float kOrbRadius           = 26.0f;
inline constexpr float kMagnetDuration      = 8.0f;
inline constexpr float kMagnetRadius        = 280.0f;
inline constexpr float kShieldDuration      = 12.0f;   // shield absorbs one hit; expires if unused
inline constexpr float kTurboDuration       = 5.0f;
inline constexpr float kTurboSpeedMult      = 1.55f;
inline constexpr float kTurboMagnetRadius   = 360.0f;
inline constexpr float kHitGrace            = 1.4f;    // invulnerability after a shield break / revive

// UI: popups sit this many design units above screen centre (keeps them clear of the banner ad).
inline constexpr float kPopupLift = 46.0f;

// Revive.
inline constexpr int   kReviveCost          = 100;
inline constexpr float kReviveCountdown     = 5.0f;

// Daily reward (index = streak day, wraps after the last entry).
inline constexpr int kDailyRewards[] = {50, 75, 100, 150, 200, 300, 500};
inline constexpr int kDailyRewardCount = sizeof(kDailyRewards) / sizeof(kDailyRewards[0]);

// Progression: level = 1 + floor(sqrt(xp / kXpPerLevelUnit)); level-up pays kLevelUpCoins * level.
inline constexpr float kXpPerLevelUnit   = 120.0f;
inline constexpr int   kLevelUpCoins     = 25;
inline constexpr float kXpPerDistance    = 0.1f;
inline constexpr int   kXpPerCoin        = 2;
inline constexpr int   kXpPerStage       = 40;

// Missions ------------------------------------------------------------------------------------
enum class MissionType {
    RunDistance,   // reach `target` distance in a single run
    RunCoins,      // collect `target` coins in a single run
    TotalCoins,    // collect `target` coins across today's runs
    PlayRuns,      // finish `target` runs today
    NearMisses,    // `target` near misses today
    PowerUps,      // collect `target` power-ups today
    ReachStage,    // reach stage `target` in a single run
    ComboChain,    // reach a coin combo of `target` in a single run
};

struct MissionTemplate {
    MissionType type;
    int target;
    int reward;
    const char* text;   // printf with the target
};

inline constexpr MissionTemplate kMissionPool[] = {
    {MissionType::RunDistance, 400,  40, "RUN %dM IN ONE GO"},
    {MissionType::RunDistance, 900,  90, "RUN %dM IN ONE GO"},
    {MissionType::RunCoins,     30,  35, "GRAB %d COINS IN A RUN"},
    {MissionType::TotalCoins,  120,  50, "COLLECT %d COINS TODAY"},
    {MissionType::PlayRuns,      5,  30, "PLAY %d RUNS"},
    {MissionType::NearMisses,    6,  45, "%d CLOSE CALLS TODAY"},
    {MissionType::PowerUps,      3,  50, "PICK UP %d POWER-UPS"},
    {MissionType::ReachStage,    3,  60, "REACH STAGE %d"},
    {MissionType::ComboChain,   10,  45, "HIT A x%d COIN COMBO"},
};
inline constexpr int kMissionPoolCount = sizeof(kMissionPool) / sizeof(kMissionPool[0]);
inline constexpr int kMissionsPerDay   = 3;

// ---------------------------------------------------------------------------------------------
// Optional art overrides (relative to Content/Images/HD). Used only when the file exists.
// ---------------------------------------------------------------------------------------------
inline const char* kArtOrbMagnet = "powerup_magnet.png";
inline const char* kArtOrbShield = "powerup_shield.png";
inline const char* kArtOrbTurbo  = "powerup_turbo.png";

} // namespace PZCfg
