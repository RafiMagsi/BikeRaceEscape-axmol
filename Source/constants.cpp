//
//  constants.c
//  ProjectMayhem
//
//  Created by Nik Rudenko on 2/8/13.
//
//

#include <stdio.h>
#include "Application.h"
#include "Director.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <android/log.h>
#endif

float kDeviceScale(){
    ax::TargetPlatform target =  ax::Application::sharedApplication()->getTargetPlatform();
    ax::Size s = ax::Director::getInstance()->getWinSize();

    if ( target == ax::kTargetIpad ) {
        return 1.0;
    }
    else if (target == ax::kTargetMacOS){
        if (s.height >= 640) {
            return 1.0;
        }
        else{
            return 0.5;
        }
    }
    else if(target == ax::kTargetAndroid){
        if(s.height >= 480){
            return 1.0;
        }
        else{
            return 0.5;
        }
    }

    return 0.5;
}

ax::Size kDeviceWinSize(){
    return ax::Director::getInstance()->getWinSize();
}

float kPhysicsWorldScale(){
     ax::TargetPlatform target =  ax::Application::sharedApplication()->getTargetPlatform();
    if ( target == ax::kTargetIpad ) {
        return 0.05;
    }
    else if (target == ax::kTargetMacOS){
        ax::Size s = ax::Director::getInstance()->getWinSize();
        if (s.height >= 640) {
            return 0.05;
        }
        else{
            return 0.1;
        }
    }
    else if(target == ax::kTargetAndroid){
        return 0.1;
    }
    
    return 0.1;
    
}

void PZLog( const char* msg){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    __android_log_write(ANDROID_LOG_DEBUG, "PZPlayer", msg);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ISO)
    printf("[PZPlayer] - %s\n", msg);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    printf("[PZPlayer] - %s\n", msg);    
#endif
}