#include "Ads/AdsController.h"

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
}

void AdsController::onMainMenu() { handleContext(AdsContext::MainMenu); }
void AdsController::onLevelsMenu() { handleContext(AdsContext::LevelsMenu); }
void AdsController::onPauseMenu() { handleContext(AdsContext::PauseMenu); }
void AdsController::onPlaying() { handleContext(AdsContext::Playing); }
void AdsController::onGameOver() { handleContext(AdsContext::GameOver); }

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

void AdsController::handleContext(AdsContext ctx) {
    const int i = static_cast<int>(ctx);
    if (!adsEnabled_ || !provider_) return;

    // Banner schedule
    const auto& s = config_.setups[i];
    if (s.bannerAdMobWeight > 0 && s.bannerShowAfterCount > 0) {
        bannerCounters_[i] += 1;
        if (bannerCounters_[i] >= s.bannerShowAfterCount) {
            // Only show if we have a configured banner id.
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
            const std::string id = config_.admob.bannerIdIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            const std::string id = config_.admob.bannerIdAndroid;
#else
            const std::string id{};
#endif
            if (!id.empty()) {
                loadBanner({id});
                showBanner();
            }
            bannerCounters_[i] = 0;
        }
    } else {
        // Default: hide banner for contexts not configured.
        hideBanner();
    }

    // Interstitial schedule (best-effort; provider decides readiness)
    if (s.interstitialAdMobWeight > 0 && s.interstitialShowAfterCount > 0) {
        interstitialCounters_[i] += 1;
        if (interstitialCounters_[i] >= s.interstitialShowAfterCount) {
            AdRequest req;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
            req.placementId = config_.admob.interstitialIdIOS;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            req.placementId = config_.admob.interstitialIdAndroid;
#endif
            if (!req.placementId.empty()) {
                loadInterstitial(req);
                showInterstitial();
            }
            interstitialCounters_[i] = 0;
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
