//
//  V2Outcome.h
//  Result headline for the Midnight Crown story (PRODUCT_PLAN 24.5): ONE primary outcome picked by priority, one best mastery
//  moment and the next reachable target. Pure logic (no nodes) so the priority order is testable. All text comes from
//  CompleteGameInfo.plist (v2.presentation.dialogues: outcome_*, moment_*, next_*), formatted here with the numbers.
//
#pragma once

#include <string>

namespace V2 {
namespace Outcome2 {

struct Inputs {
    bool trexEscaped = false;       // survived the T-Rex chase this run
    bool firstCrown = false;        // ... for the first time ever
    bool rivalBeaten = false;       // won the Cool Kid race this run
    bool newDistanceRecord = false;
    int  dailyMedal = 0;            // medal earned by a daily run this run (0 none)
    bool friendBeaten = false;      // friend challenge: your score beat the link's score
    int  distance = 0, best = 0;
    int  smooth = 0, closeDodges = 0, eventsCleared = 0, microDone = 0;
    int  perfect = 0, bestChain = 0;
    std::string newStamp;           // "event/tier" first clean success this run ("" none)
};

struct Headline {
    int priority = 5;               // 1 crown .. 5 night ended
    std::string title;              // "CROWN CLAIMED"
    std::string moment;             // best mastery moment, "" when there was none
    std::string next;               // next target
    int momentum = 0;               // CROWN MOMENTUM 0-100: one presentation value built from landings, chains, dodges, events, rival, T-Rex (a summary, not a currency)
};

Headline pick(const Inputs& in);
std::string selfTest();

} // namespace Outcome2
} // namespace V2
