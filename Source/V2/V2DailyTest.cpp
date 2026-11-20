//
//  V2DailyTest.cpp
//  In-app rule tests for the daily challenge (PRODUCT_PLAN 20.8 #4): attempts, best retention, reward-once, medal upgrades,
//  forfeits, modifiers / thresholds, date rollover, legacy-save migration and the no-double-count scoring rule.
//  Runs against UserDefault on far-future dates (year 2098/2099), restores the upload queue and deletes its own keys.
//  Call V2::Daily::selfTest() from a debug hook; it returns a multi-line report ending in "RESULT: PASS" or "RESULT: FAIL".
//
#include "V2/V2Daily.h"
#include "V2/V2Config.h"
#include "PZLegacyCompat.h"

#include <string>

USING_NS_AX;

namespace V2 {
namespace Daily {

namespace {
struct Report {
    std::string text;
    int fails = 0, checks = 0;
    void check(bool ok, const std::string& what) {
        ++checks;
        if (!ok) ++fails;
        text += std::string(ok ? "  ok   " : "  FAIL ") + what + "\n";
    }
};

void wipe(int date) {
    auto* ud = UserDefault::getInstance();
    for (const char* sfx : {"_n", "_r", "_c", "_p", "_v"}) ud->deleteValueForKey(("PZP_D" + std::to_string(date) + sfx).c_str());
}

Result ranked(int date, int distance, int coins = 0) {
    Result r;
    r.dateKey = date;
    r.mode = Mode::Ranked;
    r.distance = distance;
    r.coins = coins;
    return r;
}
}

std::string selfTest() {
    Report R;
    auto* ud = UserDefault::getInstance();
    const std::string savedQueue(ud->getStringForKey("PZP_DQ", ""));
    const Config& c = cfg();
    const int D1 = 20990101, D2 = 20990102, D3 = 20990103, D4 = 20990104, D5 = 20980101;
    for (int d : {D1, D2, D3, D4, D5}) wipe(d);

    // 1. three ranked attempts, then practice
    {
        int g = 0;
        R.check(attemptsLeft(D1) == c.attemptsPerDay, "fresh day has all attempts");
        for (int i = 0; i < c.attemptsPerDay; ++i) record(ranked(D1, 10 + i), g);
        R.check(attemptsLeft(D1) == 0 && attemptsUsed(D1) == c.attemptsPerDay, "attempts used up after N ranked results");
        const int before = rankedResult(D1).score;
        record(ranked(D1, 99999), g);
        R.check(attemptsUsed(D1) == c.attemptsPerDay, "4th result does not use a ranked attempt");
        R.check(rankedResult(D1).score == before, "4th result (practice) cannot replace the ranked best");
        R.check(bestPractice(D1) > before, "practice best is stored separately");
    }
    // 2. best retention
    {
        int g = 0;
        record(ranked(D2, 800), g);
        const int best = rankedResult(D2).score;
        record(ranked(D2, 50), g);
        R.check(rankedResult(D2).score == best, "a worse ranked attempt keeps the best");
        record(ranked(D2, 1500), g);
        R.check(rankedResult(D2).score > best, "a better ranked attempt replaces the best");
    }
    // 3. reward paid once, then only medal upgrades
    {
        int g1 = 0, g2 = 0, g3 = 0, g4 = 0;
        record(ranked(D3, 0), g1);
        R.check(g1 == c.rewardByMedal[0], "first ranked result pays the participation reward");
        record(ranked(D3, 0), g2);
        R.check(g2 == 0, "same medal again pays nothing");
        const int m = record(ranked(D3, 100000, 5000), g3);
        R.check(m == 3 && g3 == c.rewardByMedal[3] - c.rewardByMedal[0], "gold pays only the difference");
        // attempts are now used up (3): further results are practice and pay nothing
        record(ranked(D3, 100000, 5000), g4);
        R.check(g4 == 0, "practice results never pay");
    }
    // 4. forfeit (on a far-future "today" so real saves are never touched)
    {
        const int base = debugDayOffset();
        setDebugDayOffset(base + 3000);
        begin(Mode::Ranked);
        const int today = dateKeyNow();
        wipe(today);
        Result live = ranked(today, 120);
        snapshot(live);
        Result f;
        R.check(takeForfeit(f) && f.forfeited, "leaving a live ranked run yields a forfeit");
        R.check(!takeForfeit(f), "a forfeit can only be taken once");
        int g = 0;
        record(f, g);
        R.check(attemptsUsed(today) == 1, "a forfeit uses one ranked attempt");
        wipe(today);
        end();
        R.check(mode() == Mode::None, "end() returns to normal play");
        setDebugDayOffset(base);
    }
    // 5. modifiers and thresholds
    {
        bool seen[6] = {false, false, false, false, false, false};
        bool ordered = true, mono = true;
        for (int day = 0; day < 40; ++day) {
            const int date = 20990200 + day;
            const Modifier m = modifierFor(date);
            if (m.index >= 0 && m.index < 6) seen[m.index] = true;
            ordered = ordered && medalThreshold(1, m) < medalThreshold(2, m) && medalThreshold(2, m) < medalThreshold(3, m);
            mono = mono && medalFor(medalThreshold(3, m), m) == 3 && medalFor(medalThreshold(1, m), m) == 1 && medalFor(medalThreshold(1, m) - 1, m) == 0;
        }
        bool all = true;
        for (bool b : seen) all = all && b;
        R.check(all, "all six modifiers occur within 40 consecutive dates");
        R.check(ordered, "medal thresholds ascend for every modifier");
        R.check(mono, "medalFor agrees with medalThreshold at the boundaries");
        R.check(seedFor(D1) == seedFor(D1) && seedFor(D1) != seedFor(D2), "seed is stable per date and differs between dates");
    }
    // 6. no double counting of event coins
    {
        const Modifier std0 = modifierFor(D4);
        Result a;
        a.distance = 300; a.coins = 100; a.eventCoins = 40; a.eventsCleared = 2;
        Result b = a;
        b.coins = 60; b.eventCoins = 0;
        R.check(computeScore(a, std0) == computeScore(b, std0), "event coins are excluded from the coin part of the score");
        Result z;
        R.check(computeScore(z, std0) >= 0, "an empty run never scores below zero");
    }
    // 7. date rollover
    {
        const int base = debugDayOffset();
        const int d0 = dateKeyNow();
        setDebugDayOffset(base + 1);
        const int d1 = dateKeyNow();
        R.check(d1 > d0, "debug day offset advances the date key");
        wipe(d1);
        R.check(attemptsLeft(d1) == c.attemptsPerDay, "a new day starts with a full set of attempts");
        setDebugDayOffset(base);
    }
    // 8. legacy (rules v1) save migration
    {
        ud->setStringForKey(("PZP_D" + std::to_string(D5) + "_r").c_str(), "900,300,40,2,1,standard,0");
        ud->setBoolForKey(("PZP_D" + std::to_string(D5) + "_c").c_str(), true);
        R.check(attemptsUsed(D5) == 1, "legacy ranked day counts as one attempt used");
        R.check(rankedResult(D5).score == 900 && rankedResult(D5).medal == 1, "legacy ranked result is still readable");
        int g = 0;
        record(ranked(D5, 0), g);
        R.check(g == 0, "legacy reward is not paid a second time");
        R.check(attemptsUsed(D5) == 2 && rankedResult(D5).score >= 900, "legacy best survives the next attempt");
    }

    for (int d : {D1, D2, D3, D4, D5}) wipe(d);
    ud->setStringForKey("PZP_DQ", savedQueue);
    ud->flush();
    R.text += "CHECKS: " + std::to_string(R.checks) + "  FAILED: " + std::to_string(R.fails) + "\n";
    R.text += R.fails == 0 ? "RESULT: PASS\n" : "RESULT: FAIL\n";
    return R.text;
}

} // namespace Daily
} // namespace V2
