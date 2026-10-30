#include "Ads/AdsController.h"

#include "Ads/Providers/AdsProviderMux.h"
#include "PZLegacyCompat.h"

namespace PZ {

AdsController* AdsController::shared() {
    static AdsController instance;
    return &instance;
}

void AdsController::setProvider(std::unique_ptr<IAdsProvider> provider) {
    provider_ = std::move(provider);
    initialized_ = false;
}

void AdsController::setAdsEnabled(bool enabled) {
    adsEnabled_ = enabled;
    if (!adsEnabled_) {
        hideBanner();
    }
}

bool AdsController::initialize(const std::string& appIdOrKey) {
    appIdOrKey_ = appIdOrKey;
    if (!provider_) {
        AXLOGI("AdsController: no provider set; ads disabled by default");
        initialized_ = false;
        return false;
    }
    if (initialized_) return true;
    initialized_ = provider_->initialize(appIdOrKey_);
    AXLOGI("AdsController: initialize provider='{}' result={}", provider_->name(), initialized_);
    return initialized_;
}

void AdsController::loadConfigFromPlist() {
    config_ = AdsConfig::loadFromAdsPlist();

    // Choose provider based on config (for now: AdMob only).
    if (config_.admobEnabled) {
        // Provider must be set by the app bootstrap (keeps SDK optional).
        AXLOGI("AdsController: ads.plist enables AdMob (provider must be set by bootstrap)");
    }
    if (config_.chartboost.enabled) {
        AXLOGI("AdsController: ads.plist enables Chartboost (provider must be set by bootstrap)");
    }
}

void AdsController::onMainMenu() { handleContext(AdsContext::MainMenu); }
void AdsController::onLevelsMenu() { handleContext(AdsContext::LevelsMenu); }
void AdsController::onPauseMenu() { handleContext(AdsContext::PauseMenu); }
void AdsController::onPlaying() { handleContext(AdsContext::Playing); }
void AdsController::onGameOver() { handleContext(AdsContext::GameOver); }
void AdsController::onMoreGames() {
    if (!adsEnabled_ || !provider_) return;

    // Prefer Chartboost interstitial for the "more games/free game" button (fast full-screen).
    // If Chartboost is disabled, fall back to rewarded (if configured) or interstitial.
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    if (config_.chartboost.enabled && !config_.chartboost.interstitialLocationIOS.empty()) {
        const std::string loc = config_.chartboost.interstitialLocationIOS;
        const bool ready = provider_->isInterstitialReady();
        AXLOGI("AdsController: onMoreGames (Chartboost) ready={} location='{}'", ready, loc);
        if (ready) {
            showInterstitial();
            loadInterstitial({loc}); // warm next one
        } else {
            loadInterstitial({loc});
            AXLOGI("AdsController: onMoreGames Chartboost not ready; caching only");
        }
        return;
    }
#endif

    // Fallbacks (kept for compatibility)
    if (config_.admobEnabled) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        if (!config_.admob.rewardedIdIOS.empty()) {
            loadRewarded({config_.admob.rewardedIdIOS});
            showRewarded();
            return;
        }
        if (!config_.admob.interstitialIdIOS.empty()) {
            loadInterstitial({config_.admob.interstitialIdIOS});
            showInterstitial();
            return;
        }
#endif
    }
}

void AdsController::showBanner() {
    if (!adsEnabled_ || !provider_) return;
    provider_->showBanner();
}

void AdsController::loadBanner(const AdRequest& request) {
    if (!adsEnabled_ || !provider_) return;
    provider_->loadBanner(request);
}

void AdsController::hideBanner() {
    if (!provider_) return;
    provider_->hideBanner();
}

void AdsController::loadInterstitial(const AdRequest& request) {
    if (!adsEnabled_ || !provider_) return;
    provider_->loadInterstitial(request);
}

void AdsController::showInterstitial() {
    if (!adsEnabled_ || !provider_) return;
    provider_->showInterstitial();
}

void AdsController::loadRewarded(const AdRequest& request) {
    if (!adsEnabled_ || !provider_) return;
    provider_->loadRewarded(request);
}

void AdsController::showRewarded() {
    if (!adsEnabled_ || !provider_) return;
    provider_->showRewarded();
}

void AdsController::selectAdMobInterstitial(bool admob) {
    if (auto* mux = dynamic_cast<AdsProviderMux*>(provider_.get())) {
        mux->useBannerProviderForInterstitial(admob);
    }
}

void AdsController::handleContext(AdsContext ctx) {
    const int i = static_cast<int>(ctx);
    if (!adsEnabled_ || !provider_) return;

    const auto& s = config_.setups[i];

    // Banner (AdMob): shown on menu contexts, hidden while playing.
    {
        std::string bannerId;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        bannerId = config_.admob.bannerIdIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        bannerId = config_.admob.bannerIdAndroid;
#endif
        if (config_.admobEnabled && s.bannerAdMobWeight > 0 && !bannerId.empty()) {
            bannerCounters_[i] += 1;
            if (bannerCounters_[i] >= s.bannerShowAfterCount) {
                loadBanner({bannerId});
                showBanner();
            }
        } else {
            hideBanner();
        }
    }

    // Interstitial schedule: alternates AdMob / Chartboost when both are weighted.
    // Show only when cached (showing before ready causes a white loading screen on Chartboost),
    // then cache the next one.
    const bool admobInterstitialOn = config_.admobEnabled && s.interstitialAdMobWeight > 0;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    const bool chartboostInterstitialOn = config_.chartboost.enabled && s.interstitialChartboostWeight > 0;
#else
    const bool chartboostInterstitialOn = false; // Chartboost is only wired on iOS.
#endif
    if ((admobInterstitialOn || chartboostInterstitialOn) && s.interstitialShowAfterCount > 0) {
        interstitialCounters_[i] += 1;
        if (interstitialCounters_[i] >= s.interstitialShowAfterCount) {
            interstitialCounters_[i] = 0;

            bool useAdMob = admobInterstitialOn && (!chartboostInterstitialOn || nextInterstitialAdMob_);
            auto requestFor = [&](bool admob) {
                AdRequest r;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
                r.placementId = admob ? config_.admob.interstitialIdIOS : config_.chartboost.interstitialLocationIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
                r.placementId = admob ? config_.admob.interstitialIdAndroid : std::string();
#endif
                return r;
            };

            // Prefer the scheduled network; fall back to the other one if it is not cached.
            selectAdMobInterstitial(useAdMob);
            bool ready = provider_->isInterstitialReady();
            if (!ready && admobInterstitialOn && chartboostInterstitialOn) {
                selectAdMobInterstitial(!useAdMob);
                if (provider_->isInterstitialReady()) {
                    useAdMob = !useAdMob;
                    ready = true;
                } else {
                    selectAdMobInterstitial(useAdMob);
                }
            }
            const AdRequest req = requestFor(useAdMob);
            AXLOGI("AdsController: interstitial tick (ctx={}, network={}, ready={})", i, useAdMob ? "AdMob" : "Chartboost", ready);
            if (ready) {
                showInterstitial();
                nextInterstitialAdMob_ = !useAdMob;
            }
            if (!req.placementId.empty()) loadInterstitial(req); // cache the next one
        }
    }

    // Proactive caching on Main Menu so GameOver can show instantly.
    if (ctx == AdsContext::MainMenu) {
        for (const bool admob : {true, false}) {
#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
            if (!admob) continue;
#endif
            if (admob ? !config_.admobEnabled : !config_.chartboost.enabled) continue;
            AdRequest r;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
            r.placementId = admob ? config_.admob.interstitialIdIOS : config_.chartboost.interstitialLocationIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            r.placementId = admob ? config_.admob.interstitialIdAndroid : std::string();
#endif
            if (r.placementId.empty()) continue;
            selectAdMobInterstitial(admob);
            if (!provider_->isInterstitialReady()) loadInterstitial(r);
        }
    }

    // Rewarded schedule is supported by config, but should generally be triggered by explicit UI events.
    // We keep this for compatibility with older configs that auto-show rewarded at certain contexts.
    if (s.rewardedAdMobWeight > 0 && s.rewardedShowAfterCount > 0) {
        rewardedCounters_[i] += 1;
        if (rewardedCounters_[i] >= s.rewardedShowAfterCount) {
            AdRequest req;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
            req.placementId = config_.admob.rewardedIdIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            req.placementId = config_.admob.rewardedIdAndroid;
#endif
            if (!req.placementId.empty()) {
                loadRewarded(req);
                showRewarded();
            }
            rewardedCounters_[i] = 0;
        }
    }
}

} // namespace PZ
