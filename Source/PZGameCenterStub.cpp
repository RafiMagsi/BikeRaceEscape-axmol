#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)

#include "PZGameCenter.h"

namespace PZ {

GameCenter* GameCenter::shared() {
    static GameCenter instance;
    return &instance;
}

void GameCenter::setLeaderboardId(const char*) {}
void GameCenter::playerLogin() {}
void GameCenter::showLeaderboard() {}
void GameCenter::restorePurchases() {}
void GameCenter::submitScore(int) {}

void GameCenter::showProcessingView() {}
void GameCenter::hideProcessingView() {}
void GameCenter::showErrorView(const char*) {}

} // namespace PZ

#endif // non-iOS
