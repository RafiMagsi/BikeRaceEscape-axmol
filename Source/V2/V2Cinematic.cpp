#include "V2/V2Cinematic.h"
#include "V2/V2Core.h"

namespace V2 {

Cinematic& Cinematic::shared() {
    static Cinematic c;
    return c;
}

const char* Cinematic::name(Owner o) {
    switch (o) {
    case Owner::None: return "none";
    case Owner::PreRunDialogue: return "pre_run_dialogue";
    case Owner::Intro: return "intro";
    case Owner::Gameplay: return "gameplay";
    case Owner::Climax: return "climax";
    case Owner::Blast: return "blast";
    case Owner::Death: return "death";
    case Owner::Revive: return "revive";
    }
    return "?";
}

bool Cinematic::request(Owner o) {
    if (o == _owner) return true;
    bool ok = false;
    switch (o) {
    case Owner::Death: ok = true; break;                                                       // death always wins
    case Owner::PreRunDialogue: ok = _owner == Owner::None; break;
    case Owner::Intro: ok = _owner == Owner::None || _owner == Owner::PreRunDialogue; break;
    case Owner::Gameplay: ok = _owner == Owner::None || _owner == Owner::Intro; break;
    case Owner::Climax: ok = _owner == Owner::Gameplay || _owner == Owner::None; break;
    case Owner::Blast: ok = _owner == Owner::Gameplay || _owner == Owner::None; break;
    case Owner::Revive: ok = _owner == Owner::Death || _owner == Owner::Gameplay || _owner == Owner::None; break;
    case Owner::None: ok = true; break;
    }
    if (!ok) {
        ++_denied;
        Analytics::shared().log("cinematic_denied", {{"want", name(o)}, {"owner", name(_owner)}});
        return false;
    }
    _owner = o;
    return true;
}

void Cinematic::release(Owner o) {
    if (_owner != o) return;
    switch (o) {
    case Owner::PreRunDialogue: _owner = Owner::None; break;          // the intro asks next
    case Owner::Death: _owner = Owner::None; break;                   // results / revive take over
    case Owner::Intro: case Owner::Climax: case Owner::Blast: case Owner::Revive: _owner = Owner::Gameplay; break;
    default: break;
    }
}

void Cinematic::forceReset() { _owner = Owner::None; }

std::string Cinematic::selfTest() {
    Cinematic c;
    std::string r;
    int fails = 0, checks = 0;
    auto ck = [&](bool ok, const char* what) { ++checks; if (!ok) ++fails; r += std::string(ok ? "  ok   " : "  FAIL ") + what + "\n"; };
    ck(c.request(Owner::PreRunDialogue) && c.owner() == Owner::PreRunDialogue, "dialogue can own an idle screen");
    ck(!c.request(Owner::Blast), "blast is denied during dialogue");
    c.release(Owner::PreRunDialogue);
    ck(c.request(Owner::Intro), "intro follows dialogue");
    ck(!c.allowsGameplayCamera(), "no gameplay punches during the intro");
    c.release(Owner::Intro);
    ck(c.owner() == Owner::Gameplay && c.allowsGameplayCamera(), "gameplay owns the camera after GO");
    ck(c.request(Owner::Climax), "climax starts from gameplay");
    ck(!c.request(Owner::Blast), "blast is denied during the climax");
    ck(!c.allowsGameplayCamera(), "no punches during the climax");
    ck(c.request(Owner::Death) && c.owner() == Owner::Death, "death always wins");
    c.release(Owner::Climax);
    ck(c.owner() == Owner::Death, "a non-owner cannot release");
    ck(c.request(Owner::Revive), "revive may follow death");
    c.release(Owner::Revive);
    ck(c.owner() == Owner::Gameplay, "revive hands back to gameplay");
    c.forceReset();
    ck(c.owner() == Owner::None, "forceReset always returns to none");
    ck(c.denied() == 2, "denials are counted");
    r += "CHECKS: " + std::to_string(checks) + "  FAILED: " + std::to_string(fails) + "\n";
    r += fails == 0 ? "RESULT: PASS\n" : "RESULT: FAIL\n";
    return r;
}

} // namespace V2
