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
#if  (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    Sprite *splash;
    if(FileUtils::getInstance()->isFileExist( "default.png" )){
        splash = Sprite::create( "default.png" );    
    }
#else
    Sprite* splash = nullptr;

    // Legacy project referenced a splash key ("GeneralApplicationSplashScreen_0_0") which isn't present in this port's
    // Content bundle. Avoid calling Sprite::create with a missing/blank filename (it can assert/crash on iOS).
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
            if (splash) break;
        }
    }
#endif
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

    
    this->scheduleOnce( AX_SCHEDULE_SELECTOR( PZGLoadingScene::continueLoadingSelector ), 0.5);
    
	return true;
}

void PZGLoadingScene::continueLoadingSelector( float dt){
    (void)dt;
    AXLOGI("PZGLoadingScene::continueLoadingSelector");
    AppDelegate::scheduledLoading();
}
