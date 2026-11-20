//
//  V2Daily.h
//  Daily seeded challenge: one road for everybody per UTC day, a modifier of the day, versioned scoring, medals, one
//  ranked attempt (the first finished run of the day) plus unlimited practice, a local result queue for a future
//  leaderboard upload, and an idempotent participation reward. See versions/v2/PRODUCT_PLAN.md section 7.1.
//
//  Pure logic + persistence: no scenes, no UI. The run director applies the modifier; the popups (PZPopups) show it.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace V2 {
namespace Daily {

constexpr int kRulesVersion = 1;      // bump when scoring / physics / content changes: old scores stay on their own board

enum class Mode { None, Ranked, Practice };

struct Modifier {
    std::string id;
    std::string title;
    std::string desc;
    float scoreMult = 1.f;
    bool noRevive = false;            // no revive, no mercy save
    float coinDensityScale = 1.f;     // multiplies tuning/CoinDensity for the run
    float eventBreatherScale = 1.f;   // < 1 = road events come more often
};

struct Result {
    int dateKey = 0;                  // yyyymmdd (UTC)
    Mode mode = Mode::Practice;
    int score = 0, distance = 0, coins = 0, eventsCleared = 0;
    int medal = 0;                    // 0 none, 1 bronze, 2 silver, 3 gold
    std::string modifierId;
    bool forfeited = false;           // ranked run abandoned before it finished
};

int dateKeyNow();                                 // today, UTC
std::string dateLabel(int dateKey);               // "FRI 20 NOV"
uint32_t seedFor(int dateKey);                    // same everywhere, every platform
Modifier modifierFor(int dateKey);
int computeScore(int distance, int coins, int eventsCleared, const Modifier& m);
int medalFor(int score);
int medalThreshold(int medal);                    // score needed for 1..3
int rewardFor(const Result& r);                   // participation + medal coins (ranked only)

// Persistence (UserDefault) --------------------------------------------------------------------------------------------
bool hasRanked(int dateKey);
Result rankedResult(int dateKey);
int bestPractice(int dateKey);
// Stores the result (ranked only the first time), queues it for upload and returns the coins granted now (0 if none or
// already claimed). Idempotent for the same date.
int record(const Result& r, int& coinsGranted);
std::vector<std::string> pendingUploads();        // lines "date|mode|score|rules|modifier" not yet submitted
void markUploaded(size_t count);                  // drop the first `count` queue entries

// Session (what the current run chain is doing) -------------------------------------------------------------------------
void begin(Mode m);
Mode mode();
const Modifier& activeModifier();
void switchToPractice();                          // the ranked attempt is used up: retries are practice
void end();                                       // back on the main menu: normal play again
// Live snapshot of the run in progress, so leaving mid-run still counts the ranked attempt.
void snapshot(int distance, int coins, int eventsCleared);
bool runLive();
void clearLive();
bool takeForfeit(Result& out);                    // ranked run left unfinished -> result to record (once)

int debugDayOffset();                             // QA: shift "today"
void setDebugDayOffset(int days);

} // namespace Daily
} // namespace V2
