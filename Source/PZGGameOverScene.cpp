//
//  GameOverScene.cpp
//  PZPlayer
//
//  Created by Nik Rudenko on 6/24/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#include "PZGGameOverScene.h"
#include "PZGGameFieldScene.h"
#include "constants.h"
#include "PZLegacyCompat.h"
#include "SharedData/PZGSharedData.h"

#include "PZGCoinShopScene.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    #include "PZGameCenter.h"
#endif

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "Neocortex.h"
#endif

USING_NS_AX;


Scene* PZGGameOverScene::scene()
{
	Scene *scene = Scene::create();
	PZGGameOverScene *layer = PZGGameOverScene::create();
	scene->addChild(layer);
    
	return scene;
}

bool PZGGameOverScene::init() {
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


void PZGGameOverScene::load(const char* keyName){
    PZGBaseMenuScene::load( keyName );
    PZSettingsController* sc = PZSettingsController::shared();
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    // IAP settings are optional in this port; keep game-over UI working without them.
    PZGGameInfoIAP *iapInfo = nullptr;
    if (sd) {
        if (auto* array = (__Array*) sd->gameInfoResource->objectForKey("IAPSettings"); array && array->count() > 0) {
            iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
        }
    }
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( 1 ); //INDEX 1 is for BACK BUTTON

        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGGameOverScene::menuBackCallback, this));
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 2 ); //INDEX 2 is for RESTART BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGGameOverScene::menuRestartCallback, this));
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
    PZGArtInterface *scorePositionInfo = (PZGArtInterface*)getItemByName( "GUI_DistanceLabelLocation", keyName );
    if (scorePositionInfo) {
        font = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "");
        
        font->setPosition( scorePositionInfo->getPosition() );
        font->setRotation( -scorePositionInfo->rotation );
        font->setScaleX( scorePositionInfo->scale_x);
        font->setScaleY( scorePositionInfo->scale_y);
        
        this->addChild( font, 301 );        
    }
    
    
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        gameOverSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDGameOver );
        gameOverSound->playAsSound( false );
        
        buttonClickedSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDButtonPressed );
    }
    
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::Neocortex::shared()->onGameover();
#endif
}

void PZGGameOverScene::setDistance(int distance){
    
    char cd[16];
    sprintf(cd, "%d", distance);
    if (font) {
        font->setString(cd);
    }
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    PZ::GameCenter *gc = PZ::GameCenter::shared();
    if (gc->isEnabled){
        gc->submitScore( distance );
    }
    
#endif
    
}

void PZGGameOverScene::menuBackCallback(Object* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound(false);
    }
    
    if (gameOverSound) {
        gameOverSound->stopSound();
    }
    
    Director *pDirector = Director::getInstance();
    pDirector->popScene();

    //Sound
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex( kSoundIDMainMenuMusic );
        soundObj->playAsBackgroundMusic();
    }
    
}

void PZGGameOverScene::menuRestartCallback(Object* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound(false);
    }

    if (gameOverSound) {
        gameOverSound->stopSound();
    }

    
    ((PZGGameFieldScene*)baseScene)->gameRestart();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex( kSoundIDGameFieldMusic );
        soundObj->playAsBackgroundMusic();
    }
}

#pragma mark IAP


void PZGGameOverScene::upgradeComplete(){
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



void PZGGameOverScene::kidModePurchased(){
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



void PZGGameOverScene::keyBackClicked(){
    menuBackCallback( NULL );
}
