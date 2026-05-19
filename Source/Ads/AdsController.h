#pragma once

#include <memory>
#include <string>

#include "IAdsProvider.h"
#include "AdsConfig.h"

namespace PZ {

class AdsController {
public:
    static AdsController* shared();

    // Provider selection/config.
    void setProvider(std::unique_ptr<IAdsProvider> provider);
    IAdsProvider* provider() const { return provider_.get(); }

    // Enable/disable ads globally (e.g. Remove Ads purchased).
    void setAdsEnabled(bool enabled);
    bool isAdsEnabled() const { return adsEnabled_; }

    // Initialize provider if any (safe to call multiple times).
    bool initialize(const std::string& appIdOrKey);

    // Load configuration from Content/ads.plist
    void loadConfigFromPlist();
    const AdsConfig& config() const { return config_; }

    // Legacy-style hooks (to be called on UI transitions)
    void onMainMenu();
    void onLevelsMenu();
    void onPauseMenu();
    void onPlaying();
    void onGameOver();

    // Banner
    void loadBanner(const AdRequest& request);
    void showBanner();
    void hideBanner();

    // Interstitial
    void loadInterstitial(const AdRequest& request);
    void showInterstitial();

    // Rewarded
    void loadRewarded(const AdRequest& request);
    void showRewarded();

private:
    AdsController() = default;

    bool adsEnabled_{true};
    bool initialized_{false};
    std::string appIdOrKey_;
    std::unique_ptr<IAdsProvider> provider_;

    AdsConfig config_{};
    int bannerCounters_[6]{};
    int interstitialCounters_[6]{};
    int rewardedCounters_[6]{};

    void handleContext(AdsContext ctx);
};

} // namespace PZ
