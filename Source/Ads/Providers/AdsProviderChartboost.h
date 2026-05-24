#pragma once

#include "Ads/IAdsProvider.h"

namespace PZ {

// Chartboost provider (iOS-only real implementation via ObjC++; stub on other platforms).
// Note: Chartboost uses "locations" for interstitials; we pass that via AdRequest::placementId.
class AdsProviderChartboost final : public IAdsProvider {
public:
    AdProviderKind kind() const override { return AdProviderKind::Chartboost; }
    const char* name() const override { return "Chartboost"; }

    bool initialize(const std::string& appIdOrKey) override;

    // Banner (not supported for this project)
    void loadBanner(const AdRequest& request) override;
    void showBanner() override;
    void hideBanner() override;

    // Interstitial
    bool isInterstitialReady() const override;
    void loadInterstitial(const AdRequest& request) override;
    void showInterstitial() override;

    // Rewarded (not supported yet)
    bool isRewardedReady() const override;
    void loadRewarded(const AdRequest& request) override;
    void showRewarded() override;

private:
    bool initialized_{false};
    bool interstitialReady_{false};
    bool interstitialAutoShow_{false};
    std::string appId_;
    std::string appSignature_;
    std::string lastInterstitialLocation_;
};

} // namespace PZ

