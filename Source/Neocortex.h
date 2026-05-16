#pragma once

// Legacy commercial/native integration stub.
// The original project used a proprietary Neocortex ad/analytics layer which is not part of this Axmol port.
// This header keeps the core game compiling for iOS/Android builds; it intentionally performs no work.

namespace PZ {
class Neocortex {
public:
    static inline Neocortex* shared() {
        static Neocortex instance;
        return &instance;
    }

    bool synchronized{false};

    inline void onMainMenu() {}
    inline void onPauseMenu() {}
    inline void onGameover() {}
    inline void onLevelsMenu() {}
    inline void onPlaying() {}
    inline void removeBanner() {}
};
} // namespace PZ

