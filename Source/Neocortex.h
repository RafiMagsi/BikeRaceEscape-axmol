#pragma once

// Legacy commercial/native integration stub.
// The original project used a proprietary Neocortex ad/analytics layer which is not part of this Axmol port.
// This header keeps the core game compiling for iOS/Android builds; it intentionally performs no work.

#include <atomic>

#include "Ads/AdsController.h"

namespace PZ {
namespace detail {
inline std::atomic<bool> g_neocortexStubLogged{false};
inline void logNeocortexStubOnce() {
    if (!g_neocortexStubLogged.exchange(true)) {
        AXLOGI("[Neocortex] Using stub -> AdsController (thirdparty/Neocortex is not linked)");
    }
}
} // namespace detail

class Neocortex {
public:
    static inline Neocortex* shared() {
        static Neocortex instance;
        return &instance;
    }

    bool synchronized{false};

    inline void onMainMenu() { detail::logNeocortexStubOnce(); AdsController::shared()->onMainMenu(); }
    inline void onPauseMenu() { detail::logNeocortexStubOnce(); AdsController::shared()->onPauseMenu(); }
    inline void onGameover() { detail::logNeocortexStubOnce(); AdsController::shared()->onGameOver(); }
    inline void onLevelsMenu() { detail::logNeocortexStubOnce(); AdsController::shared()->onLevelsMenu(); }
    inline void onPlaying() { detail::logNeocortexStubOnce(); AdsController::shared()->onPlaying(); }
    inline void onMoreGames() { detail::logNeocortexStubOnce(); AdsController::shared()->onMoreGames(); }
    inline void removeBanner() { detail::logNeocortexStubOnce(); AdsController::shared()->hideBanner(); }
};
} // namespace PZ
