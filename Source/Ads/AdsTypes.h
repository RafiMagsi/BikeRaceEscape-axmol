#pragma once

#include <string>

namespace PZ {

enum class AdProviderKind {
    None = 0,
    AdMob = 1,
    Stub = 999,
};

enum class AdPlacement {
    Banner = 0,
    Interstitial = 1,
    Rewarded = 2,
};

struct AdRequest {
    std::string placementId;
};

} // namespace PZ

