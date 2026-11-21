//
//  V2Config.h
//  Every Version 2 tuning value in one place, read from the `v2` section of Content/CompleteGameInfo.plist. Missing keys keep
//  the defaults below; values are clamped to safe ranges (a bad plist can never make a telegraph shorter than the reaction
//  floor or a medal threshold go backwards). Changing a value that affects scores should come with a bump of `rulesVersion`
//  so old scores stay on their own board.
//
#pragma once

#include <string>
#include <vector>

namespace V2 {

// One Night Run chapter (PRODUCT_PLAN 24.3): a distance band, its title and the road events that may start inside it.
struct Chapter {
    std::string id, title;
    float fromM = 0.f, toM = 1e9f;
    std::vector<std::string> events;           // event ids (eventName()); empty = every event
};

struct Config {
    int   rulesVersion = 2;

    // --- run rhythm ---
    float firstEventAt = 9.f;          // seconds of quiet opening
    float breatherMin = 6.f, breatherMax = 9.f;
    float telegraphFloor = 1.3f;       // minimum seconds between a warning (sign / shadow / paddle) and the hazard reaching you
    float tierStandardAt = 150.f;      // distance (m) where `standard` variants start
    float tierExpertAt = 450.f;        // distance (m) where `expert` variants start

    // --- rival race ---
    float raceDrift = 18.f, raceStartLead = 190.f, raceWinLead = -70.f, raceLoseLead = 470.f, raceMaxTime = 20.f;
    float gainCoin = 22.f, gainAirPerSec = 38.f, gainNearMiss = 40.f, gainLongJump = 35.f;

    // --- rewards inside runs (coins) ---
    int rewardDino = 12, rewardHighRoller = 20, rewardConstruction = 12, rewardMeteor = 10, rewardEdgeRun = 15;
    int rewardPizza = 20, rewardPizzaBox = 3, rewardGrandma = 10, rewardOvertake = 40, rewardCloseDodge = 5;
    int rewardMicro = 10;
    int rewardClimax = 40;             // T-Rex chase finale
    int climaxAfter = 4;               // event successes before the first T-Rex chase; then every climaxAfter + 1

    // --- micro challenges ---
    float microEvery = 12.f;           // seconds of breather before a micro challenge may start
    float microTime = 20.f;            // seconds to finish it

    // --- daily ---
    int   attemptsPerDay = 3;          // ranked attempts (best one counts)
    float wDistance = 1.f, wCoins = 2.f, wEvents = 25.f;   // score = distance*wD + (coins - eventCoins)*wC + events*wE + modifier bonus
    float wSmooth = 40.f;              // Perfect Landing Day: per smooth landing
    float wBox = 15.f, wBoxMiss = 10.f;// Pizza Express: per caught / per missed box
    int   medal1 = 500, medal2 = 1200, medal3 = 2500;
    int   rewardByMedal[4] = {20, 40, 80, 150};

    // --- Night Run chapters (plist v2.nightChapters / climaxAtM / climaxEveryM) ---
    std::vector<Chapter> chapters;     // ascending by fromM; empty = one chapter with every event
    float climaxAtM = 1200.f;          // first safe window after this distance starts the T-Rex chase
    float climaxEveryM = 900.f;        // then again every this many metres (After Midnight)

    struct Mod { float scoreMult = 1.f, medalScale = 1.f, breather = 1.f, coins = 1.f; };
    Mod mods[6];                       // standard, one_life, event_rush, coin_drought, perfect_landing, pizza_express
};

const Config& cfg();                   // loads on first use
void reloadConfig();                   // tests
const char* modId(int i);

} // namespace V2
