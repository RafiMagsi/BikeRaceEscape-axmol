#pragma once

// Game Center integration.
// - iOS: implemented in `PZGameCenterApple.mm` (GameKit bridge).
// - Other platforms: implemented in `PZGameCenterStub.cpp` (no-op).

namespace PZ {

class GameCenter {
public:
    static GameCenter* shared();

    // Set once (optional). If empty, `showLeaderboard()` will show the default leaderboards UI.
    void setLeaderboardId(const char* leaderboardId);

    // Authenticate local player (silently if possible).
    void playerLogin();

    // Present Game Center leaderboard UI.
    void showLeaderboard();

    // Legacy entry point kept for compatibility with existing UI (not Game Center related).
    void restorePurchases();

    // Submit score to `leaderboardId` (or default, if not set).
    void submitScore(int score);

    // Simple UI helpers used by StoreBridge flows on iOS.
    static void showProcessingView();
    static void hideProcessingView();
    static void showErrorView(const char* message);

    bool isEnabled{false};

private:
    GameCenter() = default;
    GameCenter(const GameCenter&) = delete;
    GameCenter& operator=(const GameCenter&) = delete;
};

} // namespace PZ

