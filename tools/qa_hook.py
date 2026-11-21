#!/usr/bin/env python3
"""Temporary in-app QA driver for the iOS simulator (NEVER commit its output).

  python3 tools/qa_hook.py add      inject the hook (AppDelegate.cpp + PZRunDirector.cpp), everything between QA-HOOK markers
  python3 tools/qa_hook.py remove   strip every QA-HOOK block again (verifies nothing is left)

The hook polls /tmp/pzqa/cmd every 0.2 s (simulator apps can read host /tmp). Commands (one per write):
  play | start                 main menu Play, character select Start
  event <id>[:variant]         force a road event (e.g. event trex_chase, event dino_crossing:2)
  dist <metres>                jump the run distance (enter later Night Run chapters)
  god                          toggle invulnerability (obstacles + road-event hits absorbed)
  ranked | day <n> | replay [path]   daily mode, debug day offset, replay test
  selftest                     run the in-app rule tests, report in /tmp/pzqa/selftest.txt
Files: /tmp/pzqa/bot (exists = autopilot hops gaps while the world is not frozen).
"""
import sys

APP = "Source/AppDelegate.cpp"
DIR = "Source/Features/PZRunDirector.cpp"
B, E = "// QA-HOOK-BEGIN", "// QA-HOOK-END"

HOOK = r'''
    // QA-HOOK-BEGIN
    director->getScheduler()->schedule([](float) {
        Scene* sc = Director::getInstance()->getRunningScene();
        if (!sc) return;
        PZGGamefieldBasicRunner* runner = nullptr;
        std::function<void(Node*)> walk = [&](Node* n) { if (runner) return; if (auto* r = dynamic_cast<PZGGamefieldBasicRunner*>(n)) { runner = r; return; } for (auto* c : n->getChildren()) walk(c); };
        walk(sc);
        FILE* f = fopen("/tmp/pzqa/cmd", "r"); if (!f) return;
        char buf[256] = {0}; fgets(buf, sizeof(buf), f); fclose(f); remove("/tmp/pzqa/cmd");
        std::string cmd(buf); while (!cmd.empty() && (cmd.back() == '\n' || cmd.back() == ' ')) cmd.pop_back();
        if (cmd == "play") { std::function<void(Node*)> w2 = [&](Node* n) { if (auto* m = dynamic_cast<PZGMainMenuScene*>(n)) { m->menuStartGameCallback(nullptr); return; } for (auto* c : n->getChildren()) w2(c); }; w2(sc); }
        else if (cmd == "start") { std::function<void(Node*)> w2 = [&](Node* n) { if (auto* m = dynamic_cast<PZGCharacterMenu*>(n)) { m->startGameCallback(nullptr); return; } for (auto* c : n->getChildren()) w2(c); }; w2(sc); }
        else if (cmd.rfind("event ", 0) == 0 && runner && runner->run) runner->run->forceRoadEvent(cmd.substr(6));
        else if (cmd.rfind("dist ", 0) == 0 && runner) runner->gf_distance = atof(cmd.c_str() + 5);
        else if (cmd == "god") { extern bool g_qaGod; g_qaGod = !g_qaGod; }
        else if (cmd == "selftest") { const std::string rep = V2::Daily::selfTest() + V2::Cinematic::shared().selfTest() + V2::Challenge::selfTest() + V2::Outcome2::selfTest(); FILE* o = fopen("/tmp/pzqa/selftest.txt", "w"); if (o) { fputs(rep.c_str(), o); fclose(o); } }
        else if (cmd.rfind("day ", 0) == 0) V2::Daily::setDebugDayOffset(atoi(cmd.c_str() + 4));
        else if (cmd == "ranked") V2::Daily::begin(V2::Daily::Mode::Ranked);
        else if (cmd.rfind("replay", 0) == 0) { if (cmd.size() > 7) PZRunDirector::startReplayTest(cmd.substr(7)); else PZRunDirector::startReplayTest(); }
    }, director->getScheduler(), 0.2f, false, "pzqa");
    director->getScheduler()->schedule([](float dt) {
        static float cd = 0.f, hold = 0.f;
        static PZGGamefieldBasicRunner* last = nullptr;
        cd -= dt;
        if (hold > 0.f) { hold -= dt; if (hold <= 0.f && last) last->injectInput(false); }
        FILE* f = fopen("/tmp/pzqa/bot", "r"); const bool on = f != nullptr; if (f) fclose(f);
        if (!on || cd > 0.f) return;
        Scene* sc = Director::getInstance()->getRunningScene();
        if (!sc) return;
        PZGGamefieldBasicRunner* runner = nullptr;
        std::function<void(Node*)> walk = [&](Node* n) { if (runner) return; if (auto* r = dynamic_cast<PZGGamefieldBasicRunner*>(n)) { runner = r; return; } for (auto* c : n->getChildren()) walk(c); };
        walk(sc);
        if (!runner || !runner->run || !runner->character || runner->gameOver || runner->run->frozen()) return;
        auto* pl = runner->platformLayer();
        const float x = runner->character->loc.x;
        bool danger = false;
        float y = 0.f;
        for (float d : {80.f, 120.f, 160.f, 200.f}) if (pl && !pl->groundYAt(x + d, y)) danger = true;
        if (danger && runner->character->jump_counter == 0) { runner->injectInput(true); last = runner; hold = 0.42f; cd = 0.9f; }
    }, director->getScheduler(), 0.f, false, "pzqabot");
    // QA-HOOK-END
'''
INC = '#include "PZGGamefieldBasicRunner.h" // QA-HOOK\n#include "Features/PZRunDirector.h" // QA-HOOK\n#include "V2/V2Daily.h" // QA-HOOK\n#include "V2/V2Cinematic.h" // QA-HOOK\n#include "V2/V2Challenge.h" // QA-HOOK\n#include "V2/V2Outcome.h" // QA-HOOK\n'


def strip(s):
    out, skip = [], False
    for line in s.split("\n"):
        if B in line: skip = True; continue
        if E in line: skip = False; continue
        if skip or "// QA-HOOK" in line and line.strip().startswith("#include") or "QA-TEMP" in line:
            continue
        out.append(line)
    return "\n".join(out)


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ("add", "remove"):
        print(__doc__); return 1
    a, d = open(APP).read(), open(DIR).read()
    a, d = strip(a), strip(d)
    if sys.argv[1] == "add":
        a = a.replace("    director->runWithScene(scene);\n", "    director->runWithScene(scene);\n" + HOOK, 1)
        a = a.replace('#include "PZGGameFieldScene.h"\n', '#include "PZGGameFieldScene.h"\n' + INC, 1)
        d = d.replace("bool PZRunDirector::absorbExternalHit(const Vec2& at) {\n", "bool g_qaGod = false; // QA-TEMP\nbool PZRunDirector::absorbExternalHit(const Vec2& at) {\n    if (g_qaGod) return true; // QA-TEMP\n", 1)
        d = d.replace("bool PZRunDirector::onObstacleHit(PZGObstacle* o) {\n    if (!o) return false;\n", "bool PZRunDirector::onObstacleHit(PZGObstacle* o) {\n    if (!o) return false;\n    if (g_qaGod) { destroyObstacle(o, Style::Launch); return true; } // QA-TEMP\n", 1)
    open(APP, "w").write(a)
    open(DIR, "w").write(d)
    left = sum(x.count("QA-HOOK") + x.count("QA-TEMP") for x in (a, d))
    print(("hook added" if sys.argv[1] == "add" else "hook removed") + f" ({left} marker lines present)")
    return 0 if (sys.argv[1] == "add" or left == 0) else 1


if __name__ == "__main__":
    sys.exit(main())
