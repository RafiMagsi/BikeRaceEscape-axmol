#include "Ads/Providers/AdsProviderMux.h"

#include "PZLegacyCompat.h"

namespace PZ {

AdsProviderMux::AdsProviderMux(std::unique_ptr<IAdsProvider> banner,
                               std::unique_ptr<IAdsProvider> interstitial,
                               std::unique_ptr<IAdsProvider> rewarded)
    : banner_(std::move(banner)),
      interstitial_(std::move(interstitial)),
      rewarded_(std::move(rewarded)) {}

bool AdsProviderMux::initialize(const std::string& appIdOrKey) {
    (void)appIdOrKey;
    bool ok = true;

    if (banner_) {
        ok = banner_->initialize(bannerInitKey_) && ok;
    }
    if (interstitial_) {
        ok = interstitial_->initialize(interstitialInitKey_) && ok;
    }
    if (rewarded_) {
        ok = rewarded_->initialize(rewardedInitKey_) && ok;
    }
    AXLOGI("AdsProviderMux: initialize ok={} (banner='{}', interstitial='{}', rewarded='{}')",
           ok,
           banner_ ? banner_->name() : "(null)",
           interstitial_ ? interstitial_->name() : "(null)",
           rewarded_ ? rewarded_->name() : "(null)");
    return ok;
}

void AdsProviderMux::loadBanner(const AdRequest& request) {
    if (banner_) banner_->loadBanner(request);
}
void AdsProviderMux::showBanner() {
    if (banner_) banner_->showBanner();
}
void AdsProviderMux::hideBanner() {
    if (banner_) banner_->hideBanner();
}

bool AdsProviderMux::isInterstitialReady() const {
    return interstitial_ ? interstitial_->isInterstitialReady() : false;
}
void AdsProviderMux::loadInterstitial(const AdRequest& request) {
    if (interstitial_) interstitial_->loadInterstitial(request);
}
void AdsProviderMux::showInterstitial() {
    if (interstitial_) interstitial_->showInterstitial();
}

bool AdsProviderMux::isRewardedReady() const {
    if (rewarded_) return rewarded_->isRewardedReady();
    // fallback
    if (interstitial_) return interstitial_->isRewardedReady();
    if (banner_) return banner_->isRewardedReady();
    return false;
}

void AdsProviderMux::loadRewarded(const AdRequest& request) {
    if (rewarded_) return rewarded_->loadRewarded(request);
    if (interstitial_) return interstitial_->loadRewarded(request);
    if (banner_) return banner_->loadRewarded(request);
}

void AdsProviderMux::showRewarded() {
    if (rewarded_) return rewarded_->showRewarded();
    if (interstitial_) return interstitial_->showRewarded();
    if (banner_) return banner_->showRewarded();
}

} // namespace PZ

