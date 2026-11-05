#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

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
    // Explicit ad request (UI button)
    void onMoreGames();

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

    // Opt-in rewarded video (e.g. revive). Independent of "Remove Ads": the player asked for it.
    // `requestRewarded()` preloads one (idempotent); `showRewardedWithCallback` reports whether the reward was
    // earned (false when nothing was ready, the ad failed, or the player closed it early).
    void requestRewarded();
    bool isRewardedAvailable() const;
    void showRewardedWithCallback(std::function<void(bool earned)> callback);
    // Called by the platform providers (game thread) when a rewarded ad finishes.
    void handleRewardedFinished(bool earned);

    // Policy pacing (AdMob "disruptive ads" rules): interstitials only at natural breaks, never in a player's first runs,
    // never back to back, and never straight after a rewarded video. Revive / gift taps call cancelPendingInterstitial().
    void cancelPendingInterstitial();
    void setInterstitialPolicy(int graceBreaks, float minGapSeconds, float delaySeconds) {
        graceBreaks_ = graceBreaks; minGapSeconds_ = minGapSeconds; delaySeconds_ = delaySeconds;
    }

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
    std::function<void(bool)> rewardedCallback_;
    bool nextInterstitialAdMob_{true};
    int graceBreaks_{2};            // natural breaks (game overs) without any interstitial at the start
    float minGapSeconds_{120.f};    // minimum time between two full-screen ads of any kind
    float delaySeconds_{1.4f};      // wait after the game-over screen appears so no tap can land on the ad by accident
    int breaksSeen_{0};
    double lastFullscreenAt_{-1.0e9};
    bool fullscreenAllowed() const;
    void markFullscreenShown();

    // "Free game" button: show the first cached interstitial, otherwise load and show the first that fills.
    bool moreGamesPending_{false};
    float moreGamesElapsed_{0.0f};

    struct InterstitialNetwork {
        bool admob;
        std::string placementId;
    };
    std::vector<InterstitialNetwork> interstitialNetworks() const;
    bool showFirstReadyInterstitial();
    void cancelMoreGames();

    void handleContext(AdsContext ctx);
    void selectAdMobInterstitial(bool admob);
};

} // namespace PZ
