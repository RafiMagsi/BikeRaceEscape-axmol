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
    auto* p = bannerForInterstitial_ ? banner_.get() : interstitial_.get();
    return p ? p->isInterstitialReady() : false;
}
void AdsProviderMux::loadInterstitial(const AdRequest& request) {
    auto* p = bannerForInterstitial_ ? banner_.get() : interstitial_.get();
    if (p) p->loadInterstitial(request);
}
void AdsProviderMux::showInterstitial() {
    auto* p = bannerForInterstitial_ ? banner_.get() : interstitial_.get();
    if (p) p->showInterstitial();
}

// Rewarded video comes from the AdMob side (banner_) unless a dedicated rewarded provider was given:
// the interstitial provider (Chartboost) has no rewarded support.
bool AdsProviderMux::isRewardedReady() const {
    auto* p = rewarded_ ? rewarded_.get() : (banner_ ? banner_.get() : interstitial_.get());
    return p ? p->isRewardedReady() : false;
}

void AdsProviderMux::loadRewarded(const AdRequest& request) {
    auto* p = rewarded_ ? rewarded_.get() : (banner_ ? banner_.get() : interstitial_.get());
    if (p) p->loadRewarded(request);
}

void AdsProviderMux::showRewarded() {
    auto* p = rewarded_ ? rewarded_.get() : (banner_ ? banner_.get() : interstitial_.get());
    if (p) p->showRewarded();
}

} // namespace PZ

