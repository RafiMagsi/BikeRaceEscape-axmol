#include "V2/V2Leaderboard.h"
#include "PZLegacyCompat.h"
#include "PZGameCenter.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif

#include <string>

USING_NS_AX;

namespace V2 {
namespace Leaderboard {

namespace {
std::string boardId() {
    static bool loaded = false;
    static std::string id;
    if (!loaded) {
        loaded = true;
        ValueMap file = FileUtils::getInstance()->getValueMapFromFile("CompleteGameInfo.plist");
        auto v2 = file.find("v2");
        if (v2 != file.end() && v2->second.getType() == Value::Type::MAP) {
            auto lb = v2->second.asValueMap().find("leaderboards");
            if (lb != v2->second.asValueMap().end() && lb->second.getType() == Value::Type::MAP) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
                const char* key = "androidDaily";
#else
                const char* key = "iosDaily";
#endif
                auto it = lb->second.asValueMap().find(key);
                if (it != lb->second.asValueMap().end()) id = it->second.asString();
            }
        }
    }
    return id;
}
}

bool configured() { return !boardId().empty(); }

void submitDaily(int score) {
    if (!configured() || score <= 0) return;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    JniHelper::callStaticVoidMethod("dev/axmol/app/PlayGamesBridge", "submitScore", boardId(), score);
#else
    PZ::GameCenter::shared()->submitScoreTo(boardId().c_str(), score);
#endif
}

void showDaily() {
    if (!configured()) return;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    JniHelper::callStaticVoidMethod("dev/axmol/app/PlayGamesBridge", "showLeaderboard", boardId());
#else
    PZ::GameCenter::shared()->showLeaderboardFor(boardId().c_str());
#endif
}

} // namespace Leaderboard
} // namespace V2
