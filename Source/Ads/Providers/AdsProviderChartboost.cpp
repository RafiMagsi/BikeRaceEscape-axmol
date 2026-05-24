#include "Ads/Providers/AdsProviderChartboost.h"

#include "PZLegacyCompat.h"

namespace PZ {

#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)

bool AdsProviderChartboost::initialize(const std::string& appIdOrKey) {
    (void)appIdOrKey;
    initialized_ = true;
    interstitialReady_ = false;
    interstitialAutoShow_ = false;
    AXLOGW("AdsProviderChartboost: iOS-only provider; running in stub mode (no ads will be shown).");
    return true;
}

void AdsProviderChartboost::loadBanner(const AdRequest&) {
    AXLOGI("AdsProviderChartboost::loadBanner (stub) - not supported");
}
void AdsProviderChartboost::showBanner() { AXLOGI("AdsProviderChartboost::showBanner (stub) - not supported"); }
void AdsProviderChartboost::hideBanner() { AXLOGI("AdsProviderChartboost::hideBanner (stub) - not supported"); }

bool AdsProviderChartboost::isInterstitialReady() const { return initialized_ && interstitialReady_; }

void AdsProviderChartboost::loadInterstitial(const AdRequest& request) {
    if (!initialized_) return;
    lastInterstitialLocation_ = request.placementId;
    interstitialReady_ = !lastInterstitialLocation_.empty();
    AXLOGI("AdsProviderChartboost::loadInterstitial (stub) location='{}'", lastInterstitialLocation_);
    if (interstitialReady_ && interstitialAutoShow_) showInterstitial();
}

void AdsProviderChartboost::showInterstitial() {
    if (!initialized_) return;
    if (!interstitialReady_) {
        interstitialAutoShow_ = true;
        AXLOGI("AdsProviderChartboost::showInterstitial (stub) not ready; will auto-show after load");
        return;
    }
    AXLOGI("AdsProviderChartboost::showInterstitial (stub) location='{}'", lastInterstitialLocation_);
    interstitialReady_ = false;
    interstitialAutoShow_ = false;
}

bool AdsProviderChartboost::isRewardedReady() const { return false; }
void AdsProviderChartboost::loadRewarded(const AdRequest&) { AXLOGI("AdsProviderChartboost::loadRewarded (stub) - not supported"); }
void AdsProviderChartboost::showRewarded() { AXLOGI("AdsProviderChartboost::showRewarded (stub) - not supported"); }

#else
// iOS implementation lives in `AdsProviderChartboostApple.mm` so we can use ObjC++.
#endif

} // namespace PZ

