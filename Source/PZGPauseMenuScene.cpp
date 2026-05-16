//
//  PauseMenuScene.cpp
//  PZPlayer
//
//  Created by Nik Rudenko on 6/29/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#include "PZGPauseMenuScene.h"
#include "PZGGameFieldScene.h"
#include "PZLegacyCompat.h"
#include "SharedData/PZGSharedData.h"
#include "PZGCoinShopScene.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "Neocortex.h"
#endif

#include "constants.h"

USING_NS_AX;


Scene* PZGPauseMenuScene::scene()
{
	Scene *scene = Scene::create();
	PZGPauseMenuScene *layer = PZGPauseMenuScene::create();
	scene->addChild(layer);
    
	return scene;
}

bool PZGPauseMenuScene::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    this->setKeypadEnabled(true);
#endif
    
    auto* bg = LayerColor::create(ccc4(0, 0, 0, 200));
    this->addChild( bg );
        
    
    
	return true;
}

void PZGPauseMenuScene::load(const char* keyName){
    PZGBaseMenuScene::load( keyName );
    
    PZSettingsController* sc = PZSettingsController::shared();
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    // IAP settings are optional in this port; keep pause menu working without them.
    PZGGameInfoIAP *iapInfo = nullptr;
    if (sd) {
        if (auto* array = (__Array*) sd->gameInfoResource->objectForKey("IAPSettings"); array && array->count() > 0) {
            iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
        }
    }
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {

        PZGArtInterface *backButtonInfo = (PZGArtInterface*)getItemByName( "GUI_BackButton", keyName );
        MenuItemSprite *item = (backButtonInfo) ? (MenuItemSprite*)menu->getChildByTag( backButtonInfo->index ) : nullptr;
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGPauseMenuScene::menuBackCallback, this));
        }

        PZGArtInterface *restartButtonInfo = (PZGArtInterface*)getItemByName( "GUI_RestartButton", keyName );
        item = (restartButtonInfo) ? (MenuItemSprite*)menu->getChildByTag( restartButtonInfo->index ) : nullptr;
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGPauseMenuScene::menuRestartCallback, this));
        }
        
        PZGArtInterface *continueButtonInfo = (PZGArtInterface*)getItemByName( "GUI_ContinueButton", keyName );
        item = (continueButtonInfo) ? (MenuItemSprite*)menu->getChildByTag( continueButtonInfo->index ) : nullptr;
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGPauseMenuScene::menuContinueCallback, this));
        }
        
        
        item = (MenuItemSprite*)menu->getChildByTag( 6 );
        if (item) {
            if (!iapInfo || sc->removeAds || iapInfo->removeAd_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::menuUpgradeCallback, this));
            }
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 5 );
        if (item) {
            if (!iapInfo || iapInfo->coinShop_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::coinShopCallback, this));
            }
        }
        item = (MenuItemSprite*)menu->getChildByTag( 4 );
        if (item) {
            if (iapInfo && iapInfo->kidMode_enabled && sc->kidMode == false) {
                item->setVisible(true);
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::buyKidModeCallback, this));
            }
            else{
                item->setVisible(false);
            }
        }
    }
    
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {      
        buttonClickedSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDButtonPressed );
    }

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::Neocortex::shared()->onPauseMenu();
#endif

}

void PZGPauseMenuScene::onEnterTransitionDidFinish(){

    
}

void PZGPauseMenuScene::menuBackCallback(Object* pSender) {
    
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound(false);
    }
    
    Director *pDirector = Director::getInstance();
    pDirector->popScene();
    
    //Sound
    
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    
    PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex( kSoundIDMainMenuMusic );
    soundObj->playAsBackgroundMusic();

}

void PZGPauseMenuScene::menuRestartCallback(Object* pSender) {
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound(false);
    }
    
    ((PZGGameFieldScene*)baseScene)->gameRestart();
    ((PZGGameFieldScene*)baseScene)->removePauseMenu();
    
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex( kSoundIDGameFieldMusic );
        soundObj->playAsBackgroundMusic();
    }
}

void PZGPauseMenuScene::menuContinueCallback(Object* pSender) {
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound(false);
    }
    
    ((PZGGameFieldScene*)baseScene)->removePauseMenu();
    CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
}


void PZGPauseMenuScene::upgradeComplete(){
    PZGBaseMenuScene::upgradeComplete();
        
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 6 ); // UPGRADE BUTTON
        if (item) {
            item->setOpacity( 0 );
            item->setEnabled( false );
        }
    }
}

void PZGPauseMenuScene::kidModePurchased(){
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }
    
    PZSettingsController *sc = PZSettingsController::shared();
    sc->kidMode = true;
    sc->kidModePurchased = true;
    sc->save();
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 4 ); //KIDMODE BUTTON
        if (item) {
            item->setVisible(false);
        }
    }
}

void PZGPauseMenuScene::keyBackClicked(){
    menuBackCallback( NULL );
}
