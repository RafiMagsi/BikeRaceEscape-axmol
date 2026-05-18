#pragma once

#include <string>

#include "PZLegacyCompat.h"

namespace PZ {

enum class AdsContext {
    BecomeActive = 0,
    MainMenu = 1,
    LevelsMenu = 2,
    PauseMenu = 3,
    Playing = 4,
    GameOver = 5,
};

struct ProviderIds {
    std::string appIdIOS;
    std::string appIdAndroid;
    std::string bannerIdIOS;
    std::string interstitialIdIOS;
    std::string rewardedIdIOS;
    std::string bannerIdAndroid;
    std::string interstitialIdAndroid;
    std::string rewardedIdAndroid;
};

struct AdsConfig {
    bool admobEnabled{false};
    ProviderIds admob;

    // per-context settings
    struct ContextSetup {
        int bannerShowAfterCount{0};
        int bannerAdMobWeight{0};
        int interstitialShowAfterCount{0};
        int interstitialAdMobWeight{0};
        int rewardedShowAfterCount{0};
        int rewardedAdMobWeight{0};
    };

    ContextSetup setups[6]{};

    static AdsConfig loadFromAdsPlist();
};

} // namespace PZ
