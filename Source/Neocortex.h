#pragma once

// Legacy commercial/native integration stub.
// The original project used a proprietary Neocortex ad/analytics layer which is not part of this Axmol port.
// This header keeps the core game compiling for iOS/Android builds; it intentionally performs no work.

#include "Ads/AdsController.h"

namespace PZ {
class Neocortex {
public:
    static inline Neocortex* shared() {
        static Neocortex instance;
        return &instance;
    }

    bool synchronized{false};

    inline void onMainMenu() { AdsController::shared()->onMainMenu(); }
    inline void onPauseMenu() { AdsController::shared()->onPauseMenu(); }
    inline void onGameover() { AdsController::shared()->onGameOver(); }
    inline void onLevelsMenu() { AdsController::shared()->onLevelsMenu(); }
    inline void onPlaying() { AdsController::shared()->onPlaying(); }
    inline void removeBanner() { AdsController::shared()->hideBanner(); }
};
} // namespace PZ
