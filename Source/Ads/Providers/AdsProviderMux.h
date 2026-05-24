#pragma once

#include <memory>

#include "Ads/IAdsProvider.h"

namespace PZ {

// Provider multiplexer:
// - Banners are forwarded to `banner_`
// - Interstitials are forwarded to `interstitial_`
// - Rewarded is forwarded to `rewarded_` (optional; falls back to interstitial_ then banner_)
//
// This lets us mix AdMob (banner) with Chartboost (interstitial) while keeping AdsController unchanged.
class AdsProviderMux final : public IAdsProvider {
public:
    AdsProviderMux(std::unique_ptr<IAdsProvider> banner,
                   std::unique_ptr<IAdsProvider> interstitial,
                   std::unique_ptr<IAdsProvider> rewarded = nullptr);

    AdProviderKind kind() const override { return AdProviderKind::Mux; }
    const char* name() const override { return "Mux"; }

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

    IAdsProvider* bannerProvider() const { return banner_.get(); }
    IAdsProvider* interstitialProvider() const { return interstitial_.get(); }
    IAdsProvider* rewardedProvider() const { return rewarded_.get(); }

    // Allow bootstrap to configure per-provider init strings.
    void setBannerInitKey(std::string k) { bannerInitKey_ = std::move(k); }
    void setInterstitialInitKey(std::string k) { interstitialInitKey_ = std::move(k); }
    void setRewardedInitKey(std::string k) { rewardedInitKey_ = std::move(k); }

private:
    std::unique_ptr<IAdsProvider> banner_;
    std::unique_ptr<IAdsProvider> interstitial_;
    std::unique_ptr<IAdsProvider> rewarded_;
    std::string bannerInitKey_;
    std::string interstitialInitKey_;
    std::string rewardedInitKey_;
};

} // namespace PZ

