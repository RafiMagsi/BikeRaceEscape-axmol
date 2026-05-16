//
//  constants.c
//  ProjectMayhem
//
//  Created by Nik Rudenko on 2/8/13.
//
//  Axmol migration note:
//  Cocos2d-x 2.x exposed Application::getTargetPlatform() plus kTarget* enums.
//  Axmol 2.11.3 no longer exposes those APIs, so the old platform routing is
//  expressed through compile-time AX/CC platform macros and the current design size.
//

#include "PZLegacyCompat.h"
#include <stdio.h>
#include "Director.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#endif

namespace {
inline ax::Size currentWindowSize() {
    return ax::Director::getInstance()->getWinSize();
}
}

float kDeviceScale(){
    const ax::Size s = currentWindowSize();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    return s.height >= 640.0f ? 1.0f : 0.5f;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    return s.height >= 480.0f ? 1.0f : 0.5f;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    // The legacy code gave iPad a 1.0 scale and iPhone 0.5.  Without the removed
    // target-platform enum, the design-height threshold preserves the same intent.
    return s.height >= 640.0f ? 1.0f : 0.5f;
#else
    return s.height >= 640.0f ? 1.0f : 0.5f;
#endif
}

ax::Size kDeviceWinSize(){
    return currentWindowSize();
}

float kPhysicsWorldScale(){
    const ax::Size s = currentWindowSize();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    return s.height >= 640.0f ? 0.05f : 0.1f;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    return 0.1f;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    return s.height >= 640.0f ? 0.05f : 0.1f;
#else
    return 0.1f;
#endif
}

void PZLog(const char* msg){
    const char* safeMsg = msg ? msg : "";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    __android_log_write(ANDROID_LOG_DEBUG, "PZPlayer", safeMsg);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    printf("[PZPlayer] - %s\n", safeMsg);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    printf("[PZPlayer] - %s\n", safeMsg);
#else
    printf("[PZPlayer] - %s\n", safeMsg);
#endif
}
