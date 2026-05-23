#include "Ads/Providers/AdsProviderAdMob.h"

#include "PZLegacyCompat.h"

namespace PZ {

#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)

bool AdsProviderAdMob::initialize(const std::string& appIdOrKey) {
    (void)appIdOrKey;
    initialized_ = true;
    interstitialReady_ = false;
    rewardedReady_ = false;
    interstitialAutoShow_ = false;
    lastBannerPlacement_.clear();
    AXLOGW("AdsProviderAdMob: SDK not integrated yet; this is a stub (no ads will be shown).");
    return true;
}

void AdsProviderAdMob::loadBanner(const AdRequest& request) {
    if (!initialized_) return;
    lastBannerPlacement_ = request.placementId;
    AXLOGI("AdsProviderAdMob::loadBanner (stub) placementId='{}'", lastBannerPlacement_);
}

void AdsProviderAdMob::showBanner() {
    if (!initialized_) return;
    AXLOGI("AdsProviderAdMob::showBanner (stub) placementId='{}'", lastBannerPlacement_);
}

void AdsProviderAdMob::hideBanner() {
    AXLOGI("AdsProviderAdMob::hideBanner (stub)");
}

bool AdsProviderAdMob::isInterstitialReady() const {
    return initialized_ && interstitialReady_;
}

void AdsProviderAdMob::loadInterstitial(const AdRequest& request) {
    if (!initialized_) return;
    lastInterstitialPlacement_ = request.placementId;
    interstitialReady_ = !lastInterstitialPlacement_.empty();
    AXLOGI("AdsProviderAdMob::loadInterstitial (stub) placementId='{}'", lastInterstitialPlacement_);
    if (interstitialReady_ && interstitialAutoShow_) {
        showInterstitial();
    }
}

void AdsProviderAdMob::showInterstitial() {
    if (!initialized_) return;
    if (!interstitialReady_) {
        interstitialAutoShow_ = true;
        AXLOGI("AdsProviderAdMob::showInterstitial (stub) not ready; will auto-show after load");
        return;
    }
    AXLOGI("AdsProviderAdMob::showInterstitial (stub) placementId='{}'", lastInterstitialPlacement_);
    interstitialReady_ = false;
    interstitialAutoShow_ = false;
}

bool AdsProviderAdMob::isRewardedReady() const {
    return initialized_ && rewardedReady_;
}

void AdsProviderAdMob::loadRewarded(const AdRequest& request) {
    if (!initialized_) return;
    lastRewardedPlacement_ = request.placementId;
    rewardedReady_ = !lastRewardedPlacement_.empty();
    AXLOGI("AdsProviderAdMob::loadRewarded (stub) placementId='{}'", lastRewardedPlacement_);
}

void AdsProviderAdMob::showRewarded() {
    if (!initialized_) return;
    if (!rewardedReady_) {
        AXLOGI("AdsProviderAdMob::showRewarded (stub) not ready");
        return;
    }
    AXLOGI("AdsProviderAdMob::showRewarded (stub) placementId='{}'", lastRewardedPlacement_);
    rewardedReady_ = false;
}

#else
// iOS implementation lives in `AdsProviderAdMobApple.mm` so we can use ObjC++ and the Google Mobile Ads SDK
// when it is present. This translation unit is intentionally empty on iOS to avoid duplicate symbols.
#endif

} // namespace PZ

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
// NOTE:
// Real iOS AdMob implementation should live in an ObjC++ file and be compiled only when
// the Google Mobile Ads SDK is present. We intentionally keep this provider as a stub
// to avoid breaking builds for contributors who haven't added the SDK yet.
#endif
