#include "V2/V2Outcome.h"
#include "V2/V2Config.h"
#include "V2/V2Content.h"

namespace V2 {
namespace Outcome2 {

namespace {
std::string fmt(std::string s, const char* key, int v) {          // replaces "{key}" with a number
    const std::string k = std::string("{") + key + "}";
    const size_t p = s.find(k);
    if (p != std::string::npos) s.replace(p, k.size(), std::to_string(v));
    return s;
}
std::string text(const char* id) { Content::shared().load(); return Content::shared().line(id); }
}

Headline pick(const Inputs& in) {
    Headline h;
    if (in.trexEscaped) { h.priority = 1; h.title = text(in.firstCrown ? "outcome_crown_claimed" : "outcome_crown_defended"); }
    else if (in.rivalBeaten) { h.priority = 2; h.title = text("outcome_rival_beaten"); }
    else if (in.newDistanceRecord) { h.priority = 3; h.title = text("outcome_distance_record"); }
    else if (in.friendBeaten) { h.priority = 4; h.title = text("outcome_friend_beaten"); }
    else if (in.dailyMedal > 0) { h.priority = 4; h.title = text("outcome_daily_medal"); }
    else { h.priority = 5; h.title = text("outcome_night_ended"); }

    // best mastery moment (data-driven wording, highest value first)
    if (in.trexEscaped) h.moment = text("moment_trex");
    else if (in.rivalBeaten) h.moment = text("moment_rival");
    else if (in.closeDodges >= 3) h.moment = fmt(text("moment_dodges"), "n", in.closeDodges);
    else if (in.smooth >= 2) h.moment = fmt(text("moment_landings"), "n", in.smooth);
    else if (in.microDone > 0) h.moment = text("moment_micro");
    else if (in.eventsCleared > 0) h.moment = fmt(text("moment_events"), "n", in.eventsCleared);

    // next reachable target: the next night chapter, else the climax, else your record
    const auto& cs = cfg().chapters;
    std::string next;
    for (const auto& c : cs) {
        if (c.fromM > static_cast<float>(in.distance)) {
            next = fmt(text("next_chapter"), "m", static_cast<int>(c.fromM));
            const size_t p = next.find("{name}");
            if (p != std::string::npos) next.replace(p, 6, c.title);
            break;
        }
    }
    if (next.empty() && static_cast<float>(in.distance) < cfg().climaxAtM) next = fmt(text("next_trex"), "m", static_cast<int>(cfg().climaxAtM));
    if (next.empty() || (in.best > in.distance && in.best - in.distance < 150)) next = fmt(text("next_record"), "m", in.best);
    h.next = next;
    return h;
}

std::string selfTest() {
    std::string r;
    int fails = 0, checks = 0;
    auto ck = [&](bool ok, const char* what) { ++checks; if (!ok) ++fails; r += std::string(ok ? "  ok   " : "  FAIL ") + what + "\n"; };
    Inputs i;
    ck(pick(i).priority == 5 && !pick(i).title.empty(), "an ordinary run ends the night");
    i.newDistanceRecord = true;
    ck(pick(i).priority == 3, "record beats an ordinary run");
    i.rivalBeaten = true;
    ck(pick(i).priority == 2, "rival beaten beats a record");
    i.trexEscaped = true; i.firstCrown = true;
    ck(pick(i).priority == 1 && pick(i).title == text("outcome_crown_claimed"), "crown beats everything");
    Inputs d; d.dailyMedal = 2;
    ck(pick(d).priority == 4, "daily medal outranks nothing but the three above");
    Inputs m; m.closeDodges = 4; m.distance = 100;
    ck(pick(m).moment.find("4") != std::string::npos, "mastery moment carries its number");
    ck(!pick(m).next.empty(), "there is always a next target");
    r += "CHECKS: " + std::to_string(checks) + "  FAILED: " + std::to_string(fails) + "\n";
    r += fails == 0 ? "RESULT: PASS\n" : "RESULT: FAIL\n";
    return r;
}

} // namespace Outcome2
} // namespace V2
