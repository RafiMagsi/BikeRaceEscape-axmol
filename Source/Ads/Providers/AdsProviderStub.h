#pragma once

#include "Ads/IAdsProvider.h"

namespace PZ {

class AdsProviderStub final : public IAdsProvider {
public:
    AdProviderKind kind() const override { return AdProviderKind::Stub; }
    const char* name() const override { return "StubAdsProvider"; }

    bool initialize(const std::string&) override { return true; }

    void loadBanner(const AdRequest&) override {}
    void showBanner() override {}
    void hideBanner() override {}

    bool isInterstitialReady() const override { return false; }
    void loadInterstitial(const AdRequest&) override {}
    void showInterstitial() override {}

    bool isRewardedReady() const override { return false; }
    void loadRewarded(const AdRequest&) override {}
    void showRewarded() override {}
};

} // namespace PZ
