
#include "PZGMainMenuScene.h"
#include "PZGCharacterMenu.h"
#include "constants.h"
#include "PZSettingsController.h"
#include "PZLegacyCompat.h"
#include "PZGGameFieldScene.h"
#include "MyObject-C-Interface.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "cocos2dx_StoreController.h"
    #include "Neocortex.h"
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    #include "PZGameCenter.h"
#endif


USING_NS_AX;

Scene* PZGMainMenuScene::scene()
{
	Scene *scene = Scene::create();
	PZGMainMenuScene *layer = PZGMainMenuScene::create();
	scene->addChild(layer);
	return scene;
}

bool PZGMainMenuScene::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    this->setKeypadEnabled(true);
#endif
    
	return true;
}

void PZGMainMenuScene::load(const char* keyName){
    AXLOGI("PZGMainMenuScene::load begin - keyName: {}", keyName ? keyName : "(null)");
    PZGBaseMenuScene::load( keyName );
    AXLOGI("PZGBaseMenuScene::load completed");

    PZSettingsController* sc = PZSettingsController::shared();

    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    __Array *array = (__Array*) sd->gameInfoResource->objectForKey("IAPSettings");
    if (!array || array->count() == 0) {
        AXLOGE("PZGMainMenuScene::load - IAPSettings array not found or empty");
        return;
    }
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex( 0 );
    if (!iapInfo) {
        AXLOGE("PZGMainMenuScene::load - IAPSettings object at index 0 is null");
        return;
    }
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( 3 ); //INDEX 3 is FOR START GAMNE BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::menuStartGameCallback, this));
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuUpgradeIAP );
        if (item) {
            if (sc->removeAds || iapInfo->removeAd_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::menuUpgradeCallback, this));
            }
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuCoinShop );
        if (item) {
            if (iapInfo->coinShop_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::coinShopCallback, this));
            }
        }
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuKidModeIAP );
        if (item) {
            if (iapInfo->kidMode_enabled && sc->kidMode == false) {
                item->setVisible(true);
                item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::buyKidModeCallback, this));
            }
            else{
                item->setVisible(false);
            }
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuKidModeOff );
        if (item) {
            if (sc->kidMode == true) {
                item->setVisible(true);
                item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::offKidModeCallback, this));
            }
            else{
                item->setVisible(false);
            }
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuGameCenter );
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::menuGameCenterCallback, this));
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            item->setVisible( false );
#endif
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuRestoreIAP ); //Restore Purchase
        if (item) {
                item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::menuRestoreCallback, this));
        }
    }
    
    
    //PLAY BACKGROUND SOUND
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    
    if (sounds) {
        PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex( kSoundIDMainMenuMusic );
        soundObj->playAsBackgroundMusic();
        
        buttonClickedSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDButtonPressed );
    }

    Size screenSize = Director::getInstance()->getWinSize();
    Sprite *more_button = Sprite::create("free_game.png" );
    Sprite *moreSpriteClick = Sprite::create( "free_game.png" );
    moreSpriteClick->setColor( ccc3(128, 128, 128) );
    MenuItemSprite* more_button1 =
        MenuItemSprite::create(more_button, moreSpriteClick, AX_CALLBACK_1(PZGMainMenuScene::moreGames, this));
    more_button1->setPosition(ccp(screenSize.width*0, screenSize.height));
    more_button1->setAnchorPoint(ccp(0, 1));
    
    Menu* pMenu;
    pMenu = Menu::create(more_button1, NULL);
    pMenu->setPosition( ccp(screenSize.width *0.9, screenSize.height*0.066) );
    //    pMenu = Menu::create(startGameItem, backItem, NULL);
    pMenu->setPosition(Vec2::ZERO);
    pMenu->setTag(400);
    
    this->addChild(pMenu,400);
    
    
}

void PZGMainMenuScene::moreGames(Object*){
//    PZ::Neocortex::shared()->showMoreFreeGames();
    ShowPlayHeaven_C();
}

void PZGMainMenuScene::onEnterTransitionDidFinish(){

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::Neocortex::shared()->onMainMenu();
#endif
    
}


void PZGMainMenuScene::update(float dd){
   
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//    PZ::GameCenter *gc = PZ::GameCenter::shared();
//    if (gc->isEnabled) {
//        gamecenterItem->setEnabled( true );
//    }
//    else{
//        gamecenterItem->setEnabled( false );
//    }
//#endif
    
}



void PZGMainMenuScene::menuStartGameCallback(Object* pSender) {

    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }

    Director *pDirector = Director::getInstance();
    Scene *pScene = nullptr;
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();

    if (!gsd || !gsd->artResource) {
        AXLOGE("menuStartGameCallback: gsd or artResource is null");
        return;
    }

    __Array* charactersArray = (__Array*)gsd->artResource->objectForKey( "Characters" );
    if (charactersArray && charactersArray->count() > 1) {
        AXLOGI("menuStartGameCallback: Loading character select menu, {} characters available", charactersArray->count());
        pScene = PZGCharacterMenu::scene();

        if (!pScene || pScene->getChildren().empty()) {
            AXLOGE("menuStartGameCallback: PZGCharacterMenu::scene() returned null or empty scene");
            return;
        }

        PZGCharacterMenu *bMenu = nullptr;

        // Try to find PZGCharacterMenu in children (it may not be at index 0)
        for (size_t i = 0; i < pScene->getChildren().size(); i++) {
            auto* child = pScene->getChildren().at(i);
            bMenu = dynamic_cast<PZGCharacterMenu*>(child);
            if (bMenu) {
                AXLOGI("menuStartGameCallback: Found PZGCharacterMenu at index {}", i);
                break;
            }
        }

        if (!bMenu) {
            AXLOGE("menuStartGameCallback: Could not find PZGCharacterMenu in {} children", pScene->getChildren().size());
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = pScene->getChildren().at(i);
                AXLOGI("  Child {}: type={}", i, typeid(*child).name());
            }
            return;
        }

        AXLOGI("menuStartGameCallback: About to call bMenu->load('InterfaceCS')");
        bMenu->load("InterfaceCS");
        AXLOGI("menuStartGameCallback: bMenu->load('InterfaceCS') completed, pScene still valid: {}", pScene != nullptr);

    }
    else {
        AXLOGI("menuStartGameCallback: Loading single character game");

        __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
        if (!array || array->count() == 0) {
            AXLOGE("menuStartGameCallback: GameplaySettings array not found or empty");
            return;
        }

        PZGGameInfoObject* gameInfoObject = (PZGGameInfoObject*)array->objectAtIndex(0);
        if (!gameInfoObject) {
            AXLOGE("menuStartGameCallback: GameplaySettings object at index 0 is null");
            return;
        }

        PZGGameFieldScene* bMenu = NULL;

        AXLOGI("CURRENT GAMEPLAY ID: {}", gameInfoObject->gameTypeId);

        if (gameInfoObject->gameTypeId == 0) {

            AXLOGI("loading: BASIC AVOIDANCE GAME");

            pScene = PZGGamefieldBasicAvoidance::scene();
            if (!pScene || pScene->getChildren().empty()) {
                AXLOGE("Failed to create PZGGamefieldBasicAvoidance scene");
                return;
            }

            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldBasicAvoidance*>(pScene->getChildren().at(i));
                if (child) {
                    bMenu = child;
                    bMenu->load("InterfaceGF");
                    break;
                }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldBasicAvoidance in scene children");
        }
        else if (gameInfoObject->gameTypeId == 1) {
            AXLOGI("loading: TILT AVOIDANCE");
            pScene = PZGGamefieldTiltAvoidance::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldTiltAvoidance*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldTiltAvoidance");
        }
        else if (gameInfoObject->gameTypeId == 2) {
            AXLOGI("loading: ROUND SHOOTER");
            pScene = PZGGamefieldRoundShooter::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldRoundShooter*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldRoundShooter");
        }
        else if (gameInfoObject->gameTypeId == 3) {
            AXLOGI("loading: SPACE SHOOTER GAME");
            pScene = PZGGamefieldSpaceShooter::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldSpaceShooter*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldSpaceShooter");
        }
        else if (gameInfoObject->gameTypeId == 4) {
            AXLOGI("loading: HELICOPTER GAME");
            pScene = PZGGamefieldHelicopterGame::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldHelicopterGame*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldHelicopterGame");
        }
        else if (gameInfoObject->gameTypeId == 5) {
            AXLOGI("loading: SLOW MOVE RUNNER");
            pScene = PZGGamefieldSlowMoveRunner::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldSlowMoveRunner*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldSlowMoveRunner");
        }
        else if (gameInfoObject->gameTypeId == 6) {
            AXLOGI("loading: BASIC RUNNER GAME");
            pScene = PZGGamefieldBasicRunner::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldBasicRunner*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldBasicRunner");
        }
        else if (gameInfoObject->gameTypeId == 7) {
            AXLOGI("loading: SHOOTING RUNNER GAME");
            pScene = PZGGamefieldShootingRunner::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldShootingRunner*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldShootingRunner");
        }
        else if (gameInfoObject->gameTypeId == 8) {
            AXLOGI("loading: SHOOTING HELICOPTER GAME");
            pScene = PZGGamefieldShootingHelicopter::scene();
            if (!pScene || pScene->getChildren().empty()) { AXLOGE("Failed to create scene"); return; }
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldShootingHelicopter*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
            if (!bMenu) AXLOGE("Failed to find PZGGamefieldShootingHelicopter");
        }

        CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
        __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
        if (sounds && sounds->count() > 8) {
            PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex(8);
            if (soundObj) soundObj->playAsBackgroundMusic();
        }
    }

    AXLOGI("menuStartGameCallback: Checking if pScene is valid before push, pScene = {}", pScene != nullptr ? "valid" : "null");

    if (pScene) {
        AXLOGI("menuStartGameCallback: About to call pDirector->pushScene()");
        pDirector->pushScene(pScene);
        AXLOGI("menuStartGameCallback: Scene pushed successfully");
    } else {
        AXLOGE("menuStartGameCallback: pScene is null, cannot push scene");
    }

    AXLOGI("menuStartGameCallback: Function exiting normally");
    
    
}

void PZGMainMenuScene::menuGameCenterCallback(Object* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    PZ::GameCenter *gc = PZ::GameCenter::shared();
    gc->playerLogin();
    if (gc->isEnabled) {
        gc->showLeaderboard();
    }
#endif
    
}

void PZGMainMenuScene::menuRestoreCallback(Object* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }

    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    PZ::GameCenter *gc = PZ::GameCenter::shared();
    gc->restorePurchases();
#endif
}


#pragma mark IAP

void PZGMainMenuScene::upgradeComplete(){
    PZGBaseMenuScene::upgradeComplete();
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 5 ); //INDEX 5 is FOR UPGRADE BUTTON
        if (item) {
            item->setOpacity( 0 );
            item->setEnabled( false );
        }
    }
}

void PZGMainMenuScene::offKidModeCallback(Object* pSender){
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }
    
    PZSettingsController *sc = PZSettingsController::shared();
    sc->kidMode = false;
    sc->save();
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuKidModeOff ); //KIDMODE OFF BUTTON
        if (item) {
            item->setVisible(false);
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuKidModeIAP ); //KIDMODE BUTTON
        if (item) {
            item->setVisible(true);
        }
    }
    
}

void PZGMainMenuScene::kidModePurchased(){
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
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuKidModeIAP ); //KIDMODE BUTTON
        if (item) {
            item->setVisible(false);
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDMainMenuKidModeOff );
        if (item) {
            if (sc->kidMode == true) {
                item->setVisible(true);
                item->setCallback(AX_CALLBACK_1(PZGMainMenuScene::offKidModeCallback, this));
            }
            else{
                item->setVisible(false);
            }
        }
    }
}

void PZGMainMenuScene::keyBackClicked(){
    Director::getInstance()->end();
}
