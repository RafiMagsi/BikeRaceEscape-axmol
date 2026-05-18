#include "Ads/Providers/AdsProviderAdMob.h"

#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#if __has_include(<GoogleMobileAds/GoogleMobileAds.h>)
#import <GoogleMobileAds/GoogleMobileAds.h>
#define PZ_HAS_GMA 1
#else
#define PZ_HAS_GMA 0
#endif

namespace PZ {

bool AdsProviderAdMob::initialize(const std::string& appIdOrKey) {
    (void)appIdOrKey;
    initialized_ = true;
    interstitialReady_ = false;
    rewardedReady_ = false;

#if PZ_HAS_GMA
    // Note: Modern GMA reads the App ID from Info.plist (GADApplicationIdentifier).
    // We keep `appIdOrKey` for compatibility with other providers / configs.
    [[GADMobileAds sharedInstance] startWithCompletionHandler:nil];
    AXLOGI("AdsProviderAdMob(iOS): Google Mobile Ads started");
    return true;
#else
    AXLOGW("AdsProviderAdMob(iOS): Google Mobile Ads SDK not found; provider runs in stub mode (no ads will be shown).");
    return true;
#endif
}

void AdsProviderAdMob::showBanner() {
    if (!initialized_) return;
#if PZ_HAS_GMA
    AXLOGW("AdsProviderAdMob(iOS): banner not implemented yet (SDK present, but we haven't wired views).");
#else
    AXLOGI("AdsProviderAdMob(iOS)::showBanner (stub)");
#endif
}

void AdsProviderAdMob::hideBanner() {
#if PZ_HAS_GMA
    AXLOGI("AdsProviderAdMob(iOS)::hideBanner (no-op)");
#else
    AXLOGI("AdsProviderAdMob(iOS)::hideBanner (stub)");
#endif
}

bool AdsProviderAdMob::isInterstitialReady() const {
    return initialized_ && interstitialReady_;
}

void AdsProviderAdMob::loadInterstitial(const AdRequest& request) {
    if (!initialized_) return;
    lastInterstitialPlacement_ = request.placementId;
    interstitialReady_ = !lastInterstitialPlacement_.empty();
#if PZ_HAS_GMA
    AXLOGW("AdsProviderAdMob(iOS): interstitial load not implemented yet (placementId='{}')", lastInterstitialPlacement_);
#else
    AXLOGI("AdsProviderAdMob(iOS)::loadInterstitial (stub) placementId='{}'", lastInterstitialPlacement_);
#endif
}

void AdsProviderAdMob::showInterstitial() {
    if (!initialized_) return;
    if (!interstitialReady_) {
        AXLOGI("AdsProviderAdMob(iOS)::showInterstitial (stub) not ready");
        return;
    }
#if PZ_HAS_GMA
    AXLOGW("AdsProviderAdMob(iOS): interstitial show not implemented yet (placementId='{}')", lastInterstitialPlacement_);
#else
    AXLOGI("AdsProviderAdMob(iOS)::showInterstitial (stub) placementId='{}'", lastInterstitialPlacement_);
#endif
    interstitialReady_ = false;
}

bool AdsProviderAdMob::isRewardedReady() const {
    return initialized_ && rewardedReady_;
}

void AdsProviderAdMob::loadRewarded(const AdRequest& request) {
    if (!initialized_) return;
    lastRewardedPlacement_ = request.placementId;
    rewardedReady_ = !lastRewardedPlacement_.empty();
#if PZ_HAS_GMA
    AXLOGW("AdsProviderAdMob(iOS): rewarded load not implemented yet (placementId='{}')", lastRewardedPlacement_);
#else
    AXLOGI("AdsProviderAdMob(iOS)::loadRewarded (stub) placementId='{}'", lastRewardedPlacement_);
#endif
}

void AdsProviderAdMob::showRewarded() {
    if (!initialized_) return;
    if (!rewardedReady_) {
        AXLOGI("AdsProviderAdMob(iOS)::showRewarded (stub) not ready");
        return;
    }
#if PZ_HAS_GMA
    AXLOGW("AdsProviderAdMob(iOS): rewarded show not implemented yet (placementId='{}')", lastRewardedPlacement_);
#else
    AXLOGI("AdsProviderAdMob(iOS)::showRewarded (stub) placementId='{}'", lastRewardedPlacement_);
#endif
    rewardedReady_ = false;
}

} // namespace PZ

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS

