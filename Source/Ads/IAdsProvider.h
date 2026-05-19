#pragma once

#include <string>

#include "AdsTypes.h"

namespace PZ {

class IAdsProvider {
public:
    virtual ~IAdsProvider() = default;

    virtual AdProviderKind kind() const = 0;
    virtual const char* name() const = 0;

    // Called once at app startup (or when switching providers).
    virtual bool initialize(const std::string& appIdOrKey) = 0;

    // Banner
    // NOTE: For banner, many providers need an ad unit id. Call `loadBanner` before `showBanner`.
    // Providers may keep the last loaded banner placement id.
    virtual void loadBanner(const AdRequest& request) = 0;
    virtual void showBanner() = 0;
    virtual void hideBanner() = 0;

    // Interstitial
    virtual bool isInterstitialReady() const = 0;
    virtual void loadInterstitial(const AdRequest& request) = 0;
    virtual void showInterstitial() = 0;

    // Rewarded
    virtual bool isRewardedReady() const = 0;
    virtual void loadRewarded(const AdRequest& request) = 0;
    virtual void showRewarded() = 0;
};

} // namespace PZ
