//
//  PZGLoadingScene.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 3/28/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGLoadingScene.h"
#include "SharedData/PZGSharedData.h"
#include "AppDelegate.h"

Scene* PZGLoadingScene::scene()
{
	Scene *scene = Scene::create();
	PZGLoadingScene *layer = PZGLoadingScene::create();
	scene->addChild(layer);
	return scene;
}

bool PZGLoadingScene::init()
{
	if ( !Layer::init() )	{
		return false;
	}

    AXLOGI("PZGLoadingScene::init");
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    (void)sd;

    // Use the same branded loading artwork on every platform. The legacy project referenced a splash key
    // ("GeneralApplicationSplashScreen_0_0") which isn't present in this port's Content bundle, and the
    // Android-only "default.png" fallback never existed either. Always initialize the pointer: leaving it
    // uninitialized caused a SIGSEGV in PZGLoadingScene::init() on Android when "default.png" was missing.
    Sprite* splash = nullptr;
    const char* candidates[] = {
        "GeneralApplicationSplashScreen_0_0.png",
        "GeneralApplicationSplashScreen.png",
        "default.png",
        "gameLogo.png",
    };

    auto* fileUtils = FileUtils::getInstance();
    for (auto* name : candidates) {
        const std::string full = fileUtils->fullPathForFilename(name);
        if (!full.empty() && fileUtils->isFileExist(full)) {
            splash = Sprite::create(full);
            if (splash) {
                AXLOGI("PZGLoadingScene: using splash image '{}'", name);
                break;
            }
        }
    }
    if (!splash) {
        AXLOGW("PZGLoadingScene: no splash image found, continuing without one");
    }
    if (splash) {
        float scale = 1.0;
        Size size = Director::getInstance()->getWinSize();
        float winAr = size.width/size.height;
        
        splash->setPosition( ccp( size.width/2.0, size.height/2.0 ) );
        Size splashSize = splash->getContentSize();

        // If the app runs in landscape but the splash asset is portrait, rotate it to match.
        // This avoids "wrong orientation" appearance when legacy assets were authored for portrait.
        const bool isLandscape = size.width > size.height;
        const bool isSplashPortrait = splashSize.height > splashSize.width;
        bool rotated = false;
        if (isLandscape && isSplashPortrait) {
            splash->setRotation(-90.0f);
            rotated = true;
        }

        const float effectiveW = rotated ? splashSize.height : splashSize.width;
        const float effectiveH = rotated ? splashSize.width : splashSize.height;
        float splashAr = effectiveW / effectiveH;
        
        if (winAr > splashAr) {
            scale = size.width / effectiveW;
        }
        else{
            scale = size.height / effectiveH;
        }
        splash->setScale( scale );
        
        this->addChild( splash );
    }

    
    // Keep the loading artwork on screen long enough to be seen. iOS already holds its LaunchScreen for
    // 2 seconds before the first frame, so Android mirrors that with the in-game loading screen.
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    constexpr float kLoadingScreenSeconds = 2.0f;
#else
    constexpr float kLoadingScreenSeconds = 0.5f;
#endif
    this->scheduleOnce( AX_SCHEDULE_SELECTOR( PZGLoadingScene::continueLoadingSelector ), kLoadingScreenSeconds);
    
	return true;
}

void PZGLoadingScene::continueLoadingSelector( float dt){
    (void)dt;
    AXLOGI("PZGLoadingScene::continueLoadingSelector");
    AppDelegate::scheduledLoading();
}
