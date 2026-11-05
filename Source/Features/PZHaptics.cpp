//
//  PZHaptics.cpp
//
#include "PZHaptics.h"
#include "PZGameConfig.h"
#include "PZProgress.h"
#include "PZLegacyCompat.h"

#include <chrono>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif

namespace PZHaptics {

void play(Kind kind) {
    PZCfg::ensureLoaded();
    if (!PZProgress::shared()->hapticsEnabled()) return;

    using clock = std::chrono::steady_clock;
    static clock::time_point last[8];
    const int k = static_cast<int>(kind);
    const auto now = clock::now();
    const float minGap = (kind == Kind::Tick) ? PZCfg::kHapticCoinInterval : 0.035f;
    if (std::chrono::duration<float>(now - last[k]).count() < minGap) return;
    last[k] = now;

    PZHaptics_native(k);
}

} // namespace PZHaptics

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
void PZHaptics_native(int kind) {
    ax::JniHelper::callStaticVoidMethod("dev/axmol/app/HapticsBridge", "play", kind);
}
#elif !(CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
void PZHaptics_native(int) {}   // desktop / other: nothing to vibrate
#endif
