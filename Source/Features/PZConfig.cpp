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
    R(theme, FontPath); R(theme, FontScale); R(theme, MinFontPx);
    R(theme, ColText); R(theme, ColDim); R(theme, ColAccent); R(theme, ColAccent2); R(theme, ColGood); R(theme, ColBad);
    R(theme, ColPanel); R(theme, ColPanelEdge); R(theme, ColPanelEdge2); R(theme, ColButton); R(theme, ColButtonDark);
    R(theme, ColMagnet); R(theme, ColShield); R(theme, ColTurbo); R(theme, ColBlast); R(theme, ColJetpack);
    R(theme, ColRocket); R(theme, ColGun);
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
    R(str, StrRocketName); R(str, StrGunName); R(str, StrRocketIcon); R(str, StrGunIcon); R(str, StrRocket); R(str, StrGun);
    R(str, StrReviveWait); R(str, StrGetReady);
    R(str, StrReviveTitle); R(str, StrReviveButton); R(str, StrReviveAdButton); R(str, StrReviveBtn); R(str, StrReviveNoCoins); R(str, StrReviveSkip); R(str, StrReviveNoCoinsShort); R(str, StrReviveCoins); R(str, StrReviveVideo); R(str, StrReviveFree); R(str, StrReviveLoading); R(str, StrReviveSub); R(str, StrMissionTrack);
    R(str, StrVideoNoAd); R(str, StrVideoRetry); R(str, StrGiftsBtn); R(str, StrGiftsTitle); R(str, StrGiftDaily); R(str, StrGiftHourly); R(str, StrGiftSpin); R(str, StrGiftVideo); R(str, StrGiftChest); R(str, StrGiftMissions); R(str, StrGiftReady); R(str, StrGiftIn); R(str, StrGiftNextChest); R(str, StrGiftMissionHint); R(str, StrGiftClaimed); R(str, StrSpinBtn); R(str, StrSpinVideo); R(str, StrSpinTitle); R(str, StrComebackTitle); R(str, StrComebackSub); R(str, StrFreeCoinsBtn); R(str, StrFreeCoinsTitle); R(str, StrFreeCoinsSub); R(str, StrFreeCoinsWait); R(str, StrFreeCoinsReady);
    R(str, StrDoubleCoins); R(str, StrDoubleClaimed); R(str, StrDoubleCoinsSub);
    R(str, StrGadgetsTitle); R(str, StrGadgetsSub); R(str, StrGadgetsBtn); R(str, StrGadgetLocked); R(str, StrGadgetEquipped); R(str, StrGadgetOwned); R(str, StrShopBtn); R(str, StrShopTitle); R(str, StrShopPowerUps); R(str, StrShopCoins); R(str, StrShopWatchAd); R(str, StrShopAdNote); R(str, StrShopPacks); R(str, StrShopBuy);
    R(str, StrDailyTitle); R(str, StrDailyDay); R(str, StrDailyClaim); R(str, StrDailyStreak); R(str, StrDailyBtn);
    R(str, StrMissionsTitle); R(str, StrMissionsBtn); R(str, StrMissionClaim); R(str, StrMissionDone);
    R(str, StrMissionNew); R(str, StrClose);
    R(str, StrResultsBest); R(str, StrResultsCoins); R(str, StrResultsDistance); R(str, StrResultsLevel); R(str, StrLevelUp);
    R(str, StrStatsDistance); R(str, StrStatsCoins); R(str, StrStatsBest);
    R(str, StrMissionToast); R(str, StrLevelPlate); R(str, StrXpFormat); R(str, StrNextUnlock); R(str, StrAllUnlocked); R(str, StrNotifYes); R(str, StrNotifNo); R(str, StrNotifOn); R(str, StrNotifBtn); R(str, StrNotifDenied); R(str, StrVibration); R(str, StrOn); R(str, StrOff);

    const ValueMap* feat = section(*root, "features");
    R(feat, EnableNotifications); R(feat, EnablePowerUps); R(feat, EnableRevive); R(feat, EnableReviveVideo); R(feat, EnableStages); R(feat, EnableCombos); R(feat, EnableNearMiss);
    R(feat, EnableDailyReward); R(feat, EnableMissions); R(feat, EnableShake); R(feat, EnableHaptics); R(feat, EnableFx);
    R(feat, EnableFairPlacement); R(feat, EnableHazardBudget); R(feat, EnableCoinArcs); R(feat, EnableMercy); R(feat, EnableJumpBuffer);
    R(feat, EnableGadgets); R(feat, EnableJetpack); R(feat, EnableFreeCoins); R(feat, EnableFreeGameAd); R(feat, EnableDoubleCoins);
    R(feat, EnableBigAir); R(feat, EnableStartGrace);
    R(feat, EnableRocket); R(feat, EnableGun);

    const ValueMap* tun = section(*root, "tuning");
    R(tun, StageLength); R(tun, StageCoinBonus);
    R(tun, ComboWindow); R(tun, ComboBonusEvery); R(tun, ComboBonusCoins);
    R(tun, NearMissRange); R(tun, NearMissCoins);
    R(tun, PowerUpFirstDelay); R(tun, PowerUpIntervalMin); R(tun, PowerUpIntervalMax);
    R(tun, OrbHeightAboveGround); R(tun, OrbRadius);
    R(tun, MagnetDuration); R(tun, MagnetRadius); R(tun, ShieldDuration); R(tun, ShieldBreakRadius);
    R(tun, TurboDuration); R(tun, TurboSpeedMult); R(tun, TurboMagnetRadius);
    R(tun, BlastRadius); R(tun, BlastMaxTargets); R(tun, BlastZoom); R(tun, BlastZoomIn); R(tun, BlastHold); R(tun, BlastGap); R(tun, BlastZoomOut); R(tun, BlastWaveSpeed); R(tun, HitGrace);
    R(tun, WeightMagnet); R(tun, WeightShield); R(tun, WeightTurbo); R(tun, WeightBlast);
    R(tun, DensityStart); R(tun, DensityFullDistance); R(tun, SafeLanding); R(tun, SafeTakeoff);
    R(tun, Budget1Distance); R(tun, Budget2Distance); R(tun, Budget3Distance); R(tun, ClusterMaxWidth); R(tun, ClusterGap); R(tun, ArcCoinCount); R(tun, ArcHeight);
    R(tun, NotifMinRuns); R(tun, NotifAskCooldownDays); R(tun, NotifMaxAsks); R(tun, JumpBuffer); R(tun, ReadySeconds); R(tun, MercyBestBelow); R(tun, MercySaves); R(tun, ReviveTurbo);
    R(tun, StartGrace); R(tun, BigAirMin); R(tun, BigAirCoins);
    R(tun, GadgetSlots); R(tun, GadgetStartStock); R(tun, GadgetMagnetSeconds); R(tun, GadgetShieldSeconds); R(tun, GadgetHeadStartM); R(tun, GadgetHelmetSaves);
    R(tun, JetpackDuration); R(tun, JetpackHeight); R(tun, JetpackCoinEvery); R(tun, JetpackCoinsPerTick); R(tun, WeightJetpack);
    R(tun, RocketDuration); R(tun, RocketSpeedMult); R(tun, GunDuration); R(tun, GunInterval); R(tun, GunRange);
    R(tun, OrbWeightRocket); R(tun, OrbWeightGun);
    R(tun, ReviveCost); R(tun, ReviveCountdown); R(tun, BannerReserve); R(tun, SmashCoins);
    R(tun, HitStop); R(tun, PunchZoom); R(tun, DustPuffs); R(tun, HapticCoinInterval);
    R(tun, XpPerLevelUnit); R(tun, LevelUpCoins); R(tun, XpPerDistance); R(tun, XpPerCoin); R(tun, XpPerStage);

    const ValueMap* gifts = section(*root, "gifts");
    R(gifts, EnableGifts); R(gifts, HourlyCooldown); R(gifts, HourlyMin); R(gifts, HourlyMax);
    R(gifts, ChestEveryLevels); R(gifts, ChestCoinsPerLevel); R(gifts, MissionChestCoins);
    R(gifts, ComebackMinHours); R(gifts, ComebackCoinsPerDay); R(gifts, ComebackMaxDays);
    auto readInts = [](const Value* v, std::vector<int>& out) {
        if (!v || v->getType() != Value::Type::VECTOR) return;
        std::vector<int> tmp;
        for (const auto& e : v->asValueVector()) tmp.push_back(e.asInt());
        if (!tmp.empty()) out = tmp;
    };
    readInts(find(gifts, "SpinRewards"), kSpinRewards);
    readInts(find(gifts, "SpinWeights"), kSpinWeights);
    if (kSpinWeights.size() != kSpinRewards.size()) kSpinWeights.assign(kSpinRewards.size(), 1);

    const ValueMap* rewards = section(*root, "rewards");
    R(rewards, FreeCoinsAmount); R(rewards, FreeCoinsCooldown); R(rewards, DoubleCoinsMultiplier);

    const ValueMap* notif = section(*root, "notifications");
    auto readMessages = [](const Value& v) {
        std::vector<NotifMessage> out;
        if (v.getType() != Value::Type::VECTOR) return out;
        for (const auto& row : v.asValueVector()) {
            if (row.getType() != Value::Type::MAP) continue;
            const auto& m = row.asValueMap();
            auto t = m.find("Title"), b = m.find("Body");
            if (t == m.end() || b == m.end()) continue;
            out.push_back({t->second.asString(), b->second.asString()});
        }
        return out;
    };
    if (auto* a = find(notif, "Asks")) {
        auto v = readMessages(*a);
        if (!v.empty()) kNotifAsks = v;
    }
    if (auto* sl = find(notif, "Slots"); sl && sl->getType() == Value::Type::VECTOR) {
        std::vector<NotifSlot> slots;
        for (const auto& row : sl->asValueVector()) {
            if (row.getType() != Value::Type::MAP) continue;
            const auto& m = row.asValueMap();
            NotifSlot slot;
            if (auto it = m.find("Id"); it != m.end()) slot.id = it->second.asString();
            if (auto it = m.find("Hours"); it != m.end()) slot.hours = it->second.asFloat();
            if (auto it = m.find("LocalHour"); it != m.end()) slot.localHour = it->second.asInt();
            if (auto it = m.find("MinStreak"); it != m.end()) slot.minStreak = it->second.asInt();
            if (auto it = m.find("Messages"); it != m.end()) slot.messages = readMessages(it->second);
            if (!slot.id.empty() && slot.hours > 0.f && !slot.messages.empty()) slots.push_back(slot);
        }
        if (!slots.empty()) kNotifSlots = slots;
    }

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
            auto lv = g.find("UnlockLevel"), val = g.find("Value"), price = g.find("Price");
            if (id == g.end() || name == g.end() || icon == g.end()) continue;
            GadgetTemplate t;
            t.id = id->second.asString();
            t.name = name->second.asString();
            t.desc = desc != g.end() ? desc->second.asString() : "";
            t.icon = icon->second.asString();
            t.unlockLevel = lv != g.end() ? std::max(1, lv->second.asInt()) : 1;
            t.value = val != g.end() ? val->second.asFloat() : 0.f;
            t.price = price != g.end() ? std::max(0, price->second.asInt()) : 100;
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
