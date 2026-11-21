#include "V2/V2Config.h"
#include "PZLegacyCompat.h"

#include <algorithm>

USING_NS_AX;

namespace V2 {

namespace {
Config g_cfg;
bool g_loaded = false;

const char* kMods[6] = {"standard", "one_life", "event_rush", "coin_drought", "perfect_landing", "pizza_express"};

void defaults(Config& c) {
    c = Config();
    c.mods[0] = {1.0f, 1.0f, 1.0f, 1.0f};
    c.mods[1] = {1.5f, 1.0f, 1.0f, 1.0f};
    c.mods[2] = {1.25f, 1.0f, 0.5f, 1.0f};
    c.mods[3] = {1.3f, 0.8f, 1.0f, 0.4f};
    c.mods[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    c.mods[5] = {1.0f, 1.0f, 1.0f, 1.0f};
}

float getF(const ValueMap& m, const char* k, float def) {
    auto it = m.find(k);
    return it == m.end() ? def : it->second.asFloat();
}
int getI(const ValueMap& m, const char* k, int def) {
    auto it = m.find(k);
    return it == m.end() ? def : it->second.asInt();
}
float clampf(float v, float a, float b) { return std::max(a, std::min(b, v)); }
}

const char* modId(int i) { return kMods[std::max(0, std::min(5, i))]; }

void reloadConfig() {
    defaults(g_cfg);
    g_loaded = true;
    ValueMap file = FileUtils::getInstance()->getValueMapFromFile("CompleteGameInfo.plist");
    auto it = file.find("v2");
    if (it == file.end() || it->second.getType() != Value::Type::MAP) {
        AXLOGI("[V2:config] no v2 section in CompleteGameInfo.plist, using defaults");
        return;
    }
    const ValueMap& m = it->second.asValueMap();
    Config& c = g_cfg;
#define F(name, lo, hi) c.name = clampf(getF(m, #name, c.name), lo, hi)
#define I(name, lo, hi) c.name = std::max(lo, std::min(hi, getI(m, #name, c.name)))
    I(rulesVersion, 1, 999);
    F(firstEventAt, 3.f, 60.f); F(breatherMin, 2.f, 30.f); F(breatherMax, 2.f, 40.f); F(telegraphFloor, 1.0f, 3.f);
    F(tierStandardAt, 20.f, 2000.f); F(tierExpertAt, 40.f, 5000.f);
    F(raceDrift, 4.f, 60.f); F(raceStartLead, 80.f, 400.f); F(raceWinLead, -300.f, 0.f); F(raceLoseLead, 200.f, 900.f); F(raceMaxTime, 8.f, 60.f);
    F(gainCoin, 0.f, 120.f); F(gainAirPerSec, 0.f, 200.f); F(gainNearMiss, 0.f, 160.f); F(gainLongJump, 0.f, 160.f);
    I(rewardDino, 0, 500); I(rewardHighRoller, 0, 500); I(rewardConstruction, 0, 500); I(rewardMeteor, 0, 500); I(rewardEdgeRun, 0, 500);
    I(rewardPizza, 0, 500); I(rewardPizzaBox, 0, 100); I(rewardGrandma, 0, 500); I(rewardOvertake, 0, 500); I(rewardCloseDodge, 0, 100);
    I(rewardMicro, 0, 500);
    I(rewardClimax, 0, 1000); I(climaxAfter, 2, 30);
    F(microEvery, 4.f, 90.f); F(microTime, 10.f, 40.f);
    I(attemptsPerDay, 1, 10);
    F(wDistance, 0.f, 10.f); F(wCoins, 0.f, 20.f); F(wEvents, 0.f, 200.f); F(wSmooth, 0.f, 200.f); F(wBox, 0.f, 100.f); F(wBoxMiss, 0.f, 100.f);
    I(medal1, 50, 100000); I(medal2, 50, 200000); I(medal3, 50, 400000);
    for (int i = 0; i < 4; ++i) c.rewardByMedal[i] = std::max(0, std::min(5000, getI(m, ("rewardMedal" + std::to_string(i)).c_str(), c.rewardByMedal[i])));
    for (int i = 0; i < 6; ++i) {
        const std::string id = kMods[i];
        c.mods[i].scoreMult = clampf(getF(m, (id + "_scoreMult").c_str(), c.mods[i].scoreMult), 0.5f, 3.f);
        c.mods[i].medalScale = clampf(getF(m, (id + "_medalScale").c_str(), c.mods[i].medalScale), 0.3f, 3.f);
        c.mods[i].breather = clampf(getF(m, (id + "_breather").c_str(), c.mods[i].breather), 0.3f, 3.f);
        c.mods[i].coins = clampf(getF(m, (id + "_coins").c_str(), c.mods[i].coins), 0.f, 2.f);
    }
#undef F
#undef I
    c.climaxAtM = clampf(getF(m, "climaxAtM", c.climaxAtM), 200.f, 20000.f);
    c.climaxEveryM = clampf(getF(m, "climaxEveryM", c.climaxEveryM), 300.f, 20000.f);
    c.chapters.clear();
    auto nc = m.find("nightChapters");
    if (nc != m.end() && nc->second.getType() == Value::Type::VECTOR) {
        for (const Value& v : nc->second.asValueVector()) {
            if (v.getType() != Value::Type::MAP) continue;
            const ValueMap& cm = v.asValueMap();
            Chapter ch;
            auto gs = [&](const char* k) { auto it = cm.find(k); return it == cm.end() ? std::string() : it->second.asString(); };
            ch.id = gs("id");
            ch.title = gs("title");
            ch.fromM = getF(cm, "fromM", 0.f);
            ch.toM = getF(cm, "toM", 1e9f);
            auto ev = cm.find("events");
            if (ev != cm.end() && ev->second.getType() == Value::Type::VECTOR) {
                for (const Value& e : ev->second.asValueVector()) ch.events.push_back(e.asString());
            }
            if (!ch.id.empty()) c.chapters.push_back(ch);
        }
        std::sort(c.chapters.begin(), c.chapters.end(), [](const Chapter& a, const Chapter& b) { return a.fromM < b.fromM; });
    }
    // consistency: ordered ranges
    if (c.breatherMax < c.breatherMin) c.breatherMax = c.breatherMin;
    if (c.tierExpertAt < c.tierStandardAt + 20.f) c.tierExpertAt = c.tierStandardAt + 20.f;
    if (c.medal2 <= c.medal1) c.medal2 = c.medal1 + 1;
    if (c.medal3 <= c.medal2) c.medal3 = c.medal2 + 1;
    AXLOGI("[V2:config] loaded v2 section (rules v{})", c.rulesVersion);
}

const Config& cfg() {
    if (!g_loaded) reloadConfig();
    return g_cfg;
}

} // namespace V2
