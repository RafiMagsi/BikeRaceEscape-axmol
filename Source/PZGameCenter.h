#pragma once

// Legacy Game Center stub.
// The original project used a native iOS Game Center layer; this port keeps the gameplay compiling without it.

namespace PZ {
class GameCenter {
public:
    static inline GameCenter* shared() {
        static GameCenter instance;
        return &instance;
    }

    bool isEnabled{false};

    inline void playerLogin() {}
    inline void showLeaderboard() {}
    inline void restorePurchases() {}
    inline void submitScore(int) {}

    static inline void showProcessingView() {}
    static inline void hideProcessingView() {}
    static inline void showErrorView(const char*) {}
};
} // namespace PZ
