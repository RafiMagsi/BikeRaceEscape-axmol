//
//  V2Daily.h
//  Daily seeded challenge: one road for everybody per UTC day, a rule modifier of the day, versioned scoring with a visible
//  breakdown, medals, best-of-N ranked attempts (default 3, then unlimited practice), a local result queue for a future
//  leaderboard upload, and an idempotent reward (paid once per date, upgraded only when the medal improves).
//  See versions/v2/PRODUCT_PLAN.md sections 7.1 and 20.
//
//  Pure logic + persistence: no scenes, no UI. All numbers come from V2Config (plist `v2` section).
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace V2 {
namespace Daily {

enum class Mode { None, Ranked, Practice };

struct Modifier {
    int index = 0;                    // 0 standard, 1 one_life, 2 event_rush, 3 coin_drought, 4 perfect_landing, 5 pizza_express
    std::string id, title, desc;
    float scoreMult = 1.f;
    float medalScale = 1.f;           // medal thresholds are multiplied by this
    bool noMercy = false;             // ONE LIFE: no free second-chance saves
    float coinDensityScale = 1.f;
    float eventBreatherScale = 1.f;   // < 1 = road events come more often
    bool pizzaBoost = false;          // PIZZA EXPRESS: pizza events are three times as common
};

struct Result {
    int dateKey = 0;
    Mode mode = Mode::Practice;
    int distance = 0;
    int coins = 0;                    // every coin / bonus the run collected
    int eventCoins = 0;               // coins paid by road events (not counted again in the score)
    int eventsCleared = 0;
    int smooth = 0;                   // smooth landings
    int boxesCaught = 0, boxesMissed = 0;
    int score = 0;
    int medal = 0;                    // 0 none, 1 bronze, 2 silver, 3 gold
    std::string modifierId;
    bool forfeited = false;           // a ranked run left before it finished
};

struct Breakdown {
    int distance = 0, coins = 0, events = 0, bonus = 0, base = 0, total = 0;
    float mult = 1.f;
    std::string bonusLabel;           // "" when the modifier has no bonus line
};

int dateKeyNow();                                 // today, UTC
std::string dateLabel(int dateKey);               // "FRI 20 NOV"
uint32_t seedFor(int dateKey);                    // same everywhere, every platform
Modifier modifierFor(int dateKey);
Modifier modifierByIndex(int index);              // replays re-apply the modifier they were recorded under
Breakdown breakdown(const Result& r, const Modifier& m);
int computeScore(const Result& r, const Modifier& m);
int medalThreshold(int medal, const Modifier& m); // score needed for medal 1..3 on this modifier
int medalFor(int score, const Modifier& m);

// Persistence (UserDefault) -------------------------------------------------------------------------------------------------
int attemptsUsed(int dateKey);
int attemptsLeft(int dateKey);
bool hasRanked(int dateKey);                      // at least one finished / forfeited ranked attempt
Result rankedResult(int dateKey);                 // the best ranked attempt of the day
int bestPractice(int dateKey);
// Stores the result. A ranked result uses one attempt and replaces the day's best when it is higher. `coinsGranted` is what
// this result paid now (participation once, then only medal upgrades). Returns the medal.
int record(const Result& r, int& coinsGranted);
std::vector<std::string> pendingUploads();        // lines "date|ranked|score|rules|modifier" not yet submitted
void markUploaded(size_t count);

// Session -------------------------------------------------------------------------------------------------------------------
void begin(Mode m);
// Friend challenge: play another day's road as a practice run (never ranked, never uses today's attempts).
void beginChallenge(int date, int theirScore, int theirMedal);
bool challengeActive();
int challengeScore();
int challengeMedal();
int playDate();                                   // the date whose road / modifier / ghost is being played (today, or the challenge date)
Mode mode();
const Modifier& activeModifier();
void afterRankedResult();                         // stay ranked while attempts remain, else practice
void end();                                       // back on the main menu: normal play again
void snapshot(const Result& live);                // live state of the run in progress (forfeit on leaving)
bool runLive();
void clearLive();
bool takeForfeit(Result& out);

std::string selfTest();                           // rule tests (V2DailyTest.cpp): returns a report ending in RESULT: PASS / FAIL
int debugDayOffset();
void setDebugDayOffset(int days);

} // namespace Daily
} // namespace V2
