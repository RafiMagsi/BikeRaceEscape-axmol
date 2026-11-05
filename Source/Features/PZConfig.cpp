//
//  PZConfig.cpp
//  Applies the `metaFeatures` dictionary of CompleteGameInfo.plist on top of the defaults in PZGameConfig.h.
//
//  Layout of the plist section (every key optional):
//    metaFeatures
//      theme      FontPath, Col* (3-int arrays), StageTints (array of [r,g,b,a])
//      strings    Str*
//      features   Enable*
//      tuning     everything else numeric (see PZGameConfig.h)
//      daily      Rewards (array of ints)
//      missions   Pool (array of {Type, Target, Reward, Text})
//      art        ArtOrb*
//  A key is the C++ variable name without the leading "k".
//
#include "PZGameConfig.h"
#include "PZLegacyCompat.h"

#include <algorithm>
#include <deque>
#include <utility>

USING_NS_AX;

namespace PZCfg {

namespace {

std::deque<std::string>& stringPool() {
    static std::deque<std::string> pool;
    return pool;
}

const ValueMap* section(const ValueMap& root, const char* name) {
    auto it = root.find(name);
    if (it == root.end() || it->second.getType() != Value::Type::MAP) return nullptr;
    return &it->second.asValueMap();
}

const Value* find(const ValueMap* m, const char* key) {
    if (!m) return nullptr;
    auto it = m->find(key);
    return it == m->end() ? nullptr : &it->second;
}

void rd(const ValueMap* m, const char* key, float& v) { if (auto* x = find(m, key)) v = x->asFloat(); }
void rd(const ValueMap* m, const char* key, int& v)   { if (auto* x = find(m, key)) v = x->asInt(); }
void rd(const ValueMap* m, const char* key, bool& v)  { if (auto* x = find(m, key)) v = x->asBool(); }
void rd(const ValueMap* m, const char* key, const char*& v) {
    if (auto* x = find(m, key)) {
        stringPool().push_back(x->asString());
        v = stringPool().back().c_str();
    }
}
void rd(const ValueMap* m, const char* key, Color3B& v) {
    auto* x = find(m, key);
    if (!x || x->getType() != Value::Type::VECTOR) return;
    const auto& a = x->asValueVector();
    if (a.size() < 3) return;
    v = Color3B(static_cast<uint8_t>(a[0].asInt()), static_cast<uint8_t>(a[1].asInt()), static_cast<uint8_t>(a[2].asInt()));
}

MissionType parseType(const std::string& s, bool& ok) {
    static const std::pair<const char*, MissionType> table[] = {
        {"RunDistance", MissionType::RunDistance}, {"RunCoins", MissionType::RunCoins},
        {"TotalCoins", MissionType::TotalCoins},   {"PlayRuns", MissionType::PlayRuns},
        {"NearMisses", MissionType::NearMisses},   {"PowerUps", MissionType::PowerUps},
        {"ReachStage", MissionType::ReachStage},   {"ComboChain", MissionType::ComboChain},
        {"SmashObstacles", MissionType::SmashObstacles},
    };
    for (const auto& e : table) {
        if (s == e.first) { ok = true; return e.second; }
    }
    ok = false;
    return MissionType::RunDistance;
}

} // namespace

#define R(sec, name) rd(sec, #name, k##name)

void ensureLoaded() {
    static bool done = false;
    if (done) return;
    done = true;

    ValueMap file = FileUtils::getInstance()->getValueMapFromFile("CompleteGameInfo.plist");
    const ValueMap* root = section(file, "metaFeatures");
    if (!root) {
        AXLOGI("[PZCfg] no metaFeatures section in CompleteGameInfo.plist, using built-in defaults");
        return;
    }

    const ValueMap* theme = section(*root, "theme");
    R(theme, FontPath);
    R(theme, ColText); R(theme, ColDim); R(theme, ColAccent); R(theme, ColAccent2); R(theme, ColGood); R(theme, ColBad);
    R(theme, ColPanel); R(theme, ColPanelEdge); R(theme, ColPanelEdge2); R(theme, ColButton); R(theme, ColButtonDark);
    R(theme, ColMagnet); R(theme, ColShield); R(theme, ColTurbo); R(theme, ColBlast); R(theme, ColJetpack);
    if (auto* t = find(theme, "StageTints"); t && t->getType() == Value::Type::VECTOR) {
        std::vector<StageTint> tints;
        for (const auto& row : t->asValueVector()) {
            if (row.getType() != Value::Type::VECTOR) continue;
            const auto& a = row.asValueVector();
            if (a.size() < 4) continue;
            tints.push_back({static_cast<uint8_t>(a[0].asInt()), static_cast<uint8_t>(a[1].asInt()),
                             static_cast<uint8_t>(a[2].asInt()), static_cast<uint8_t>(a[3].asInt())});
        }
        if (!tints.empty()) kStageTints = tints;
    }

    const ValueMap* str = section(*root, "strings");
    R(str, StrBest); R(str, StrNewBest); R(str, StrStage); R(str, StrStageSub); R(str, StrCloseCall); R(str, StrCombo);
    R(str, StrComboBonus); R(str, StrSmash); R(str, StrShieldBreak); R(str, StrBlast); R(str, StrSaved); R(str, StrGo);
    R(str, StrMagnet); R(str, StrShield); R(str, StrTurbo); R(str, StrBlastName); R(str, StrJetpackName);
    R(str, StrMagnetIcon); R(str, StrShieldIcon); R(str, StrTurboIcon); R(str, StrBlastIcon); R(str, StrJetpackIcon);
    R(str, StrJetpack); R(str, StrBigAir); R(str, StrStartHint); R(str, StrHeadStart); R(str, StrLuckyHelmet);
    R(str, StrReviveTitle); R(str, StrReviveBtn); R(str, StrReviveNoCoins); R(str, StrReviveSkip); R(str, StrReviveCoins); R(str, StrReviveVideo); R(str, StrReviveFree); R(str, StrReviveLoading); R(str, StrReviveSub); R(str, StrMissionTrack);
    R(str, StrVideoNoAd); R(str, StrVideoRetry); R(str, StrFreeCoinsBtn); R(str, StrFreeCoinsTitle); R(str, StrFreeCoinsSub); R(str, StrFreeCoinsWait); R(str, StrFreeCoinsReady);
    R(str, StrDoubleCoins); R(str, StrDoubleClaimed); R(str, StrDoubleCoinsSub);
    R(str, StrGadgetsTitle); R(str, StrGadgetsSub); R(str, StrGadgetsBtn); R(str, StrGadgetLocked); R(str, StrGadgetEquipped);
    R(str, StrDailyTitle); R(str, StrDailyDay); R(str, StrDailyClaim); R(str, StrDailyStreak); R(str, StrDailyBtn);
    R(str, StrMissionsTitle); R(str, StrMissionsBtn); R(str, StrMissionClaim); R(str, StrMissionDone);
    R(str, StrMissionNew); R(str, StrClose);
    R(str, StrResultsBest); R(str, StrResultsCoins); R(str, StrResultsDistance); R(str, StrResultsLevel); R(str, StrLevelUp);
    R(str, StrStatsDistance); R(str, StrStatsCoins); R(str, StrStatsBest);
    R(str, StrMissionToast); R(str, StrLevelPlate); R(str, StrVibration); R(str, StrOn); R(str, StrOff);

    const ValueMap* feat = section(*root, "features");
    R(feat, EnablePowerUps); R(feat, EnableRevive); R(feat, EnableReviveVideo); R(feat, EnableStages); R(feat, EnableCombos); R(feat, EnableNearMiss);
    R(feat, EnableDailyReward); R(feat, EnableMissions); R(feat, EnableShake); R(feat, EnableHaptics); R(feat, EnableFx);
    R(feat, EnableFairPlacement); R(feat, EnableCoinArcs); R(feat, EnableMercy); R(feat, EnableJumpBuffer);
    R(feat, EnableGadgets); R(feat, EnableJetpack); R(feat, EnableFreeCoins); R(feat, EnableDoubleCoins);
    R(feat, EnableBigAir); R(feat, EnableStartGrace);

    const ValueMap* tun = section(*root, "tuning");
    R(tun, StageLength); R(tun, StageCoinBonus);
    R(tun, ComboWindow); R(tun, ComboBonusEvery); R(tun, ComboBonusCoins);
    R(tun, NearMissRange); R(tun, NearMissCoins);
    R(tun, PowerUpFirstDelay); R(tun, PowerUpIntervalMin); R(tun, PowerUpIntervalMax);
    R(tun, OrbHeightAboveGround); R(tun, OrbRadius);
    R(tun, MagnetDuration); R(tun, MagnetRadius); R(tun, ShieldDuration); R(tun, ShieldBreakRadius);
    R(tun, TurboDuration); R(tun, TurboSpeedMult); R(tun, TurboMagnetRadius);
    R(tun, BlastRadius); R(tun, BlastWaveSpeed); R(tun, HitGrace);
    R(tun, WeightMagnet); R(tun, WeightShield); R(tun, WeightTurbo); R(tun, WeightBlast);
    R(tun, DensityStart); R(tun, DensityFullDistance); R(tun, SafeLanding); R(tun, SafeTakeoff);
    R(tun, ClusterMaxWidth); R(tun, ClusterGap); R(tun, ArcCoinCount); R(tun, ArcHeight);
    R(tun, JumpBuffer); R(tun, MercyBestBelow); R(tun, MercySaves); R(tun, ReviveTurbo);
    R(tun, StartGrace); R(tun, BigAirMin); R(tun, BigAirCoins);
    R(tun, GadgetSlots); R(tun, GadgetMagnetSeconds); R(tun, GadgetShieldSeconds); R(tun, GadgetHeadStartM); R(tun, GadgetHelmetSaves);
    R(tun, JetpackDuration); R(tun, JetpackHeight); R(tun, JetpackCoinEvery); R(tun, JetpackCoinsPerTick); R(tun, WeightJetpack);
    R(tun, ReviveCost); R(tun, ReviveCountdown); R(tun, BannerReserve); R(tun, SmashCoins);
    R(tun, HitStop); R(tun, DustPuffs); R(tun, HapticCoinInterval);
    R(tun, XpPerLevelUnit); R(tun, LevelUpCoins); R(tun, XpPerDistance); R(tun, XpPerCoin); R(tun, XpPerStage);

    const ValueMap* rewards = section(*root, "rewards");
    R(rewards, FreeCoinsAmount); R(rewards, FreeCoinsCooldown); R(rewards, DoubleCoinsMultiplier);

    const ValueMap* daily = section(*root, "daily");
    if (auto* r = find(daily, "Rewards"); r && r->getType() == Value::Type::VECTOR) {
        std::vector<int> rewards;
        for (const auto& v : r->asValueVector()) rewards.push_back(v.asInt());
        if (!rewards.empty()) kDailyRewards = rewards;
    }

    const ValueMap* missions = section(*root, "missions");
    if (auto* p = find(missions, "Pool"); p && p->getType() == Value::Type::VECTOR) {
        std::vector<MissionTemplate> pool;
        for (const auto& row : p->asValueVector()) {
            if (row.getType() != Value::Type::MAP) continue;
            const auto& m = row.asValueMap();
            auto type = m.find("Type"), target = m.find("Target"), reward = m.find("Reward"), text = m.find("Text");
            if (type == m.end() || target == m.end() || reward == m.end() || text == m.end()) continue;
            bool ok = false;
            const MissionType t = parseType(type->second.asString(), ok);
            if (!ok || target->second.asInt() <= 0) continue;
            pool.push_back({t, target->second.asInt(), reward->second.asInt(), text->second.asString()});
        }
        if (static_cast<int>(pool.size()) >= kMissionsPerDay) kMissionPool = pool;
    }

    const ValueMap* gadgets = section(*root, "gadgets");
    if (auto* p = find(gadgets, "Pool"); p && p->getType() == Value::Type::VECTOR) {
        std::vector<GadgetTemplate> pool;
        for (const auto& row : p->asValueVector()) {
            if (row.getType() != Value::Type::MAP) continue;
            const auto& g = row.asValueMap();
            auto id = g.find("Id"), name = g.find("Name"), desc = g.find("Desc"), icon = g.find("Icon");
            auto lv = g.find("UnlockLevel"), val = g.find("Value");
            if (id == g.end() || name == g.end() || icon == g.end()) continue;
            GadgetTemplate t;
            t.id = id->second.asString();
            t.name = name->second.asString();
            t.desc = desc != g.end() ? desc->second.asString() : "";
            t.icon = icon->second.asString();
            t.unlockLevel = lv != g.end() ? std::max(1, lv->second.asInt()) : 1;
            t.value = val != g.end() ? val->second.asFloat() : 0.f;
            if (!t.id.empty()) pool.push_back(std::move(t));
        }
        if (!pool.empty()) kGadgetPool = std::move(pool);
    }

    const ValueMap* art = section(*root, "art");
    R(art, ArtOrbMagnet); R(art, ArtOrbShield); R(art, ArtOrbTurbo); R(art, ArtOrbBlast); R(art, ArtOrbJetpack);

    AXLOGI("[PZCfg] loaded metaFeatures overrides from CompleteGameInfo.plist");
}

#undef R

} // namespace PZCfg
