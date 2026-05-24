#include "Ads/AdsConfig.h"

namespace PZ {

namespace {
static int readInt(ax::__Dictionary* d, const char* k, int def = 0) {
    if (!d || !k) return def;
    auto* s = d->valueForKey(k);
    if (s) return s->intValue();
    return def;
}

static bool readBool(ax::__Dictionary* d, const char* k, bool def = false) {
    if (!d || !k) return def;
    auto* s = d->valueForKey(k);
    if (s) return s->boolValue();
    return def;
}

static std::string readStr(ax::__Dictionary* d, const char* k) {
    if (!d || !k) return {};
    auto* s = d->valueForKey(k);
    if (s) return s->getCString();
    return {};
}

static ax::__Dictionary* dictFor(ax::__Dictionary* root, const char* k) {
    if (!root || !k) return nullptr;
    return dynamic_cast<ax::__Dictionary*>(root->objectForKey(k));
}

static AdsContext ctxFromKey(const std::string& k) {
    if (k == "OnBecomeActive") return AdsContext::BecomeActive;
    if (k == "OnMainMenu") return AdsContext::MainMenu;
    if (k == "OnLevelsMenu") return AdsContext::LevelsMenu;
    if (k == "OnPauseMenu") return AdsContext::PauseMenu;
    if (k == "OnPlaying") return AdsContext::Playing;
    if (k == "OnGameOver") return AdsContext::GameOver;
    return AdsContext::MainMenu;
}

static int ctxIndex(AdsContext c) { return static_cast<int>(c); }
} // namespace

AdsConfig AdsConfig::loadFromAdsPlist() {
    AdsConfig cfg;

    auto* fu = ax::FileUtils::getInstance();
    const auto full = fu->fullPathForFilename("ads.plist");
    if (full.empty()) {
        AXLOGW("AdsConfig: ads.plist not found (search paths). Ads disabled.");
        return cfg;
    }

    auto* root = ax::__Dictionary::createWithContentsOfFile(full.c_str());
    if (!root) {
        AXLOGW("AdsConfig: failed to parse ads.plist at {}", full);
        return cfg;
    }

    auto* constants = dictFor(root, "Constants");
    auto* admob = constants ? dictFor(constants, "AdMob") : nullptr;
    if (admob) {
        // Note: plist `true/false` becomes string "true/false" in our legacy Value->__String bridge.
        cfg.admobEnabled = readBool(admob, "enabled", false);
        cfg.admob.appIdIOS = readStr(admob, "appIdIOS");
        cfg.admob.appIdAndroid = readStr(admob, "appIdAndroid");
        cfg.admob.bannerIdIOS = readStr(admob, "bannerIdIOS");
        cfg.admob.interstitialIdIOS = readStr(admob, "interstitialIdIOS");
        cfg.admob.rewardedIdIOS = readStr(admob, "rewardedIdIOS");
        cfg.admob.bannerIdAndroid = readStr(admob, "bannerIdAndroid");
        cfg.admob.interstitialIdAndroid = readStr(admob, "interstitialIdAndroid");
        cfg.admob.rewardedIdAndroid = readStr(admob, "rewardedIdAndroid");
    }

    auto* chartboost = constants ? dictFor(constants, "Chartboost") : nullptr;
    if (chartboost) {
        cfg.chartboost.enabled = readBool(chartboost, "enabled", false);
        cfg.chartboost.appIdIOS = readStr(chartboost, "appIdIOS");
        cfg.chartboost.appSignatureIOS = readStr(chartboost, "appSignatureIOS");
        cfg.chartboost.interstitialLocationIOS = readStr(chartboost, "interstitialLocationIOS");
        if (cfg.chartboost.interstitialLocationIOS.empty()) {
            // Chartboost's default location is typically "Default".
            cfg.chartboost.interstitialLocationIOS = "Default";
        }
    }

    auto* setup = dictFor(root, "Setup");
    auto* banner = setup ? dictFor(setup, "Banner") : nullptr;
    auto* interstitial = setup ? dictFor(setup, "Interstitial") : nullptr;
    auto* rewarded = setup ? dictFor(setup, "Rewarded") : nullptr;

    if (banner) {
        auto keys = banner->allKeys();
        for (int i = 0; i < keys->count(); i++) {
            auto* ks = dynamic_cast<ax::__String*>(keys->objectAtIndex(i));
            if (!ks) continue;
            auto* d = dynamic_cast<ax::__Dictionary*>(banner->objectForKey(ks->getCString()));
            if (!d) continue;
            const int idx = ctxIndex(ctxFromKey(ks->getCString()));
            cfg.setups[idx].bannerShowAfterCount = readInt(d, "showAfterCount", 0);
            cfg.setups[idx].bannerAdMobWeight = readInt(d, "admobBanner", 0);
        }
    }
    if (interstitial) {
        auto keys = interstitial->allKeys();
        for (int i = 0; i < keys->count(); i++) {
            auto* ks = dynamic_cast<ax::__String*>(keys->objectAtIndex(i));
            if (!ks) continue;
            auto* d = dynamic_cast<ax::__Dictionary*>(interstitial->objectForKey(ks->getCString()));
            if (!d) continue;
            const int idx = ctxIndex(ctxFromKey(ks->getCString()));
            cfg.setups[idx].interstitialShowAfterCount = readInt(d, "showAfterCount", 0);
            cfg.setups[idx].interstitialAdMobWeight = readInt(d, "admobInterstitial", 0);
            cfg.setups[idx].interstitialChartboostWeight = readInt(d, "chartboostInterstitial", 0);
        }
    }
    if (rewarded) {
        auto keys = rewarded->allKeys();
        for (int i = 0; i < keys->count(); i++) {
            auto* ks = dynamic_cast<ax::__String*>(keys->objectAtIndex(i));
            if (!ks) continue;
            auto* d = dynamic_cast<ax::__Dictionary*>(rewarded->objectForKey(ks->getCString()));
            if (!d) continue;
            const int idx = ctxIndex(ctxFromKey(ks->getCString()));
            cfg.setups[idx].rewardedShowAfterCount = readInt(d, "showAfterCount", 0);
            cfg.setups[idx].rewardedAdMobWeight = readInt(d, "admobRewarded", 0);
        }
    }

    AXLOGI("AdsConfig: loaded ads.plist (admobEnabled={}, chartboostEnabled={})",
           cfg.admobEnabled, cfg.chartboost.enabled);
    return cfg;
}

} // namespace PZ
