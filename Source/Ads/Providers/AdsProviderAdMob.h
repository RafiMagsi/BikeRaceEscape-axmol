#pragma once

#include "Ads/IAdsProvider.h"

namespace PZ {

// AdMob provider placeholder.
// Real integration requires platform SDK setup (iOS: GoogleMobileAds, Android: play-services-ads).
class AdsProviderAdMob final : public IAdsProvider {
public:
    AdProviderKind kind() const override { return AdProviderKind::AdMob; }
    const char* name() const override { return "AdMob"; }

    bool initialize(const std::string& appIdOrKey) override;

    void loadBanner(const AdRequest& request) override;
    void showBanner() override;
    void hideBanner() override;

    bool isInterstitialReady() const override;
    void loadInterstitial(const AdRequest& request) override;
    void showInterstitial() override;

    bool isRewardedReady() const override;
    void loadRewarded(const AdRequest& request) override;
    void showRewarded() override;

private:
    bool initialized_{false};
    bool interstitialReady_{false};
    bool rewardedReady_{false};
    bool interstitialAutoShow_{false};
    std::string lastBannerPlacement_;
    std::string lastInterstitialPlacement_;
    std::string lastRewardedPlacement_;
};

} // namespace PZ
