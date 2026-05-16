//
//  CharacterMenu.cpp
//  ProjectZero
//
//  Created by Nik Rudenko on 8/24/12.
//
//

#include "PZGCharacterMenu.h"
#include "PZGGameFieldScene.h"
#include "PZSettingsController.h"
#include "PZGCoinShopScene.h"
#include "constants.h"

#include "PZLegacyCompat.h"

#include "SharedData/PZGSharedData.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "cocos2dx_StoreController.h"
    #include "Neocortex.h"
#endif


USING_NS_AX;

Scene* PZGCharacterMenu::scene()
{
    Scene *scene = Scene::create();
	PZGCharacterMenu *layer = PZGCharacterMenu::create();
	scene->addChild(layer);
	return scene;
}

bool PZGCharacterMenu::init()
{
	if ( !Layer::init() )	{
		return false;
	}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    this->setKeypadEnabled(true);
#endif
    
    player_c = 0;
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* charactersArray = (__Array*)gsd->artResource->objectForKey( "Characters" );
    if (charactersArray) {
        player_n = charactersArray->count();
    }
    else{
        player_n = 0;
    }

    this->schedule( AX_SCHEDULE_SELECTOR( PZGCharacterMenu::update ) );
    
    return true;
}

void PZGCharacterMenu::load(const char* keyName){
    AXLOGI("PZGCharacterMenu::load: Starting with keyName: {}", keyName ? keyName : "(null)");

    PZGBaseMenuScene::load( keyName );
    AXLOGI("PZGCharacterMenu::load: PZGBaseMenuScene::load completed");

    PZSettingsController* sc = PZSettingsController::shared();
    if (!sc) {
        AXLOGE("PZGCharacterMenu::load: PZSettingsController is null");
        return;
    }

    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    if (!sd) {
        AXLOGE("PZGCharacterMenu::load: PZGSharedData is null");
        return;
    }

    // IAPSettings is optional for this port; don't early-return or you'll lose all button callbacks.
    PZGGameInfoIAP *iapInfo = nullptr;
    if (auto* array = (__Array*) sd->gameInfoResource->objectForKey("IAPSettings"); array && array->count() > 0) {
        iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
    }
    if (!iapInfo) {
        AXLOGW("PZGCharacterMenu::load: IAPSettings missing; IAP buttons will be hidden");
    }

    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        
        MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( 4 ); //INDEX 4 is FOR START GAME BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCharacterMenu::startGameCallback, this));
            AXLOGI("PZGCharacterMenu::load: wired StartGame (tag=4)");
        } else {
            AXLOGW("PZGCharacterMenu::load: StartGame button (tag=4) not found");
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 5 ); //INDEX 5 is for BACK BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCharacterMenu::backCallback, this));
        } else {
            AXLOGW("PZGCharacterMenu::load: Back button (tag=5) not found");
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuLeftButton ); //INDEX 1 is for LEFT BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCharacterMenu::leftCallback, this));
        }

        item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuRightButton ); //INDEX 2 is for RIGHT BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCharacterMenu::rightCallback, this));
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 3 ); //INDEX 3 is for UNLOCK BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCharacterMenu::unlockCallback, this));
        }
        
        ///////////////////////
        item = (MenuItemSprite*)menu->getChildByTag( 13 ); //UPGRADE
        if (item) {
            if (!iapInfo || sc->removeAds || iapInfo->removeAd_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::menuUpgradeCallback, this));
            }
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 12 ); //COINSHOP
        if (item) {
            if (!iapInfo || iapInfo->coinShop_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::coinShopCallback, this));
            }
        }
        item = (MenuItemSprite*)menu->getChildByTag( 11 ); //KIDMODE
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
    else{
        AXLOGE("PZGCharacterMenu::load: menu (tag=kBaseMenuItemTag) not found; buttons won't respond");
    }
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    if (!gsd) {
        AXLOGE("PZGCharacterMenu::load: gsd is null, cannot create labels");
        return;
    }

    Size size = Director::getInstance()->getWinSize();

    AXLOGI("PZGCharacterMenu::load: Creating UI labels");

    PZGArtInterface *coinLabelPosition = getItemByName("GUI_CoinsLabelLocation", keyName);
    if (coinLabelPosition) {
        AXLOGI("PZGCharacterMenu::load: Creating coins label");
        coins_label = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
        if (coins_label) {
            coins_label->setScaleX( coinLabelPosition->scale_x);
            coins_label->setScaleY( coinLabelPosition->scale_y);
            coins_label->setPosition( coinLabelPosition->getPosition() );
            coins_label->setRotation( -coinLabelPosition->rotation );
            coins_label->setAlignment( TextHAlignment::LEFT );
            coins_label->setAnchorPoint( ccp(0, 0.5) );
            this->addChild( coins_label, 1000 );
            AXLOGI("PZGCharacterMenu::load: Coins label added");
        } else {
            AXLOGW("PZGCharacterMenu::load: Failed to create coins label");
        }
    } else {
        AXLOGW("PZGCharacterMenu::load: GUI_CoinsLabelLocation not found");
    }

    PZGArtInterface *playerNameLabelPosition = getItemByName("GUI_CharacterNameLabelLocation", keyName);
    if (playerNameLabelPosition) {
        AXLOGI("PZGCharacterMenu::load: Creating player name label");
        player_name = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
        if (player_name) {
            player_name->setScaleX( playerNameLabelPosition->scale_x);
            player_name->setScaleY( playerNameLabelPosition->scale_y);
            player_name->setPosition( playerNameLabelPosition->getPosition() );
            player_name->setRotation( -playerNameLabelPosition->rotation );
            player_name->setAlignment( TextHAlignment::CENTER );
            this->addChild( player_name, 1000 );
            AXLOGI("PZGCharacterMenu::load: Player name label added");
        } else {
            AXLOGW("PZGCharacterMenu::load: Failed to create player name label");
        }
    } else {
        AXLOGW("PZGCharacterMenu::load: GUI_CharacterNameLabelLocation not found");
    }

    PZGArtInterface *playerPriceLabelPosition = getItemByName("GUI_CharacterPriceLabelLocation", keyName);
    if (playerPriceLabelPosition) {
        AXLOGI("PZGCharacterMenu::load: Creating player price label");
        player_price = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
        if (player_price) {
            player_price->setScaleX( playerPriceLabelPosition->scale_x);
            player_price->setScaleY( playerPriceLabelPosition->scale_y);
            player_price->setPosition( playerPriceLabelPosition->getPosition() );
            player_price->setRotation( -playerPriceLabelPosition->rotation );
            player_price->setAlignment( TextHAlignment::CENTER );
            this->addChild( player_price, 1000 );
            AXLOGI("PZGCharacterMenu::load: Player price label added");
        } else {
            AXLOGW("PZGCharacterMenu::load: Failed to create player price label");
        }
    } else {
        AXLOGW("PZGCharacterMenu::load: GUI_CharacterPriceLabelLocation not found");
    }

    AXLOGI("PZGCharacterMenu::load: Getting player character location");
    playerPosition = getItemByName("GUI_CharacterLocation", keyName);
    if (!playerPosition) {
        AXLOGW("PZGCharacterMenu::load: GUI_CharacterLocation not found");
    }

    AXLOGI("PZGCharacterMenu::load: Loading character {}", player_c);
    loadCharacter( player_c );
    AXLOGI("PZGCharacterMenu::load: Completed successfully");
}

void PZGCharacterMenu::onEnterTransitionDidFinish(){
    
    if (coins_label) {
        PZSettingsController* sc = PZSettingsController::shared( );
        char c[ 16 ];
        sprintf( c, "%d", (int)sc->coins);
        coins_label->setString( c );        
    }
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::Neocortex::shared()->onLevelsMenu();
#endif
    
}


void PZGCharacterMenu::loadCharacter(int characterId){
    AXLOGI("PZGCharacterMenu::loadCharacter: Starting with characterId: {}", characterId);

    PZSettingsController *sc = PZSettingsController::shared();
    AXLOGI("PZGCharacterMenu::loadCharacter: Got settings controller");

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    if (!gsd) {
        AXLOGE("PZGCharacterMenu::loadCharacter: gsd is null");
        return;
    }
    AXLOGI("PZGCharacterMenu::loadCharacter: Got shared data");

    Size size = Director::getInstance()->getWinSize();
    AXLOGI("PZGCharacterMenu::loadCharacter: Got window size: {}x{}", size.width, size.height);

    if (player != NULL){
        AXLOGI("PZGCharacterMenu::loadCharacter: Removing old player");
        this->removeChild(player, true);
        player = NULL;
    }
    if (player_sprite != NULL) {
        AXLOGI("PZGCharacterMenu::loadCharacter: Removing old player sprite");
        this->removeChild(player_sprite, true);
        player_sprite = NULL;
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Getting Characters array");
    __Array* charactersInfoArray = (__Array*)gsd->artResource->objectForKey( "Characters" );
    if (charactersInfoArray == NULL) {
        AXLOGE("PZGCharacterMenu::loadCharacter: Characters array is null");
        return;
    }
    AXLOGI("PZGCharacterMenu::loadCharacter: Got {} characters", charactersInfoArray->count());

    if (characterId < 0 || characterId >= (int)charactersInfoArray->count()) {
        AXLOGE("PZGCharacterMenu::loadCharacter: characterId {} out of range [0, {})", characterId, charactersInfoArray->count());
        return;
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Getting character at index {}", characterId);
    PZGArtCharacter* characterInfo = ( PZGArtCharacter* )charactersInfoArray->objectAtIndex( characterId );
    if (characterInfo == NULL) {
        AXLOGE("PZGCharacterMenu::loadCharacter: characterInfo is null at index {}", characterId);
        return;
    }
    AXLOGI("PZGCharacterMenu::loadCharacter: Got character info");

    if (characterInfo->iconArtObj) {
        AXLOGI("PZGCharacterMenu::loadCharacter: Getting player sprite from iconArtObj");
        player_sprite = characterInfo->iconArtObj->getResource();
        AXLOGI("PZGCharacterMenu::loadCharacter: Got player sprite: {}", player_sprite != nullptr ? "valid" : "null");
    } else {
        AXLOGI("PZGCharacterMenu::loadCharacter: iconArtObj is null");
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Checking if player_sprite is null");
    if ( player_sprite == NULL) {
        AXLOGI("PZGCharacterMenu::loadCharacter: player_sprite is null, creating PZGCharacter");
        player = PZGCharacter::create( characterId, NULL, false );
        AXLOGI("PZGCharacterMenu::loadCharacter: PZGCharacter created: {}", player != nullptr ? "valid" : "null");
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Checking if player or player_sprite exists");
    if (player || player_sprite) {
        AXLOGI("PZGCharacterMenu::loadCharacter: Setting up character display");
        if (characterInfo) {
            if (player_name) {
                player_name->setString( characterInfo->name->getCString() );
            }
            if (characterInfo->enableIAP) {
                if (characterInfo->storeDescription) {
                    if (player_price) {
                        player_price->setString( characterInfo->storeDescription->getCString() );
                    }
                }
            }
            else{
                if (player_price) {
                    char c [32];
                    sprintf(c, "%d", characterInfo->price);
                    player_price->setString( c );
                }
            }
        }
        
        if (playerPosition) {
            if (player_sprite) {
                player_sprite->setPosition( ccp(playerPosition->position_x * size.width,
                                         playerPosition->position_y * size.height) );
                player_sprite->setRotation( -playerPosition->rotation );
                player_sprite->setScaleX( playerPosition->scale_x );
                player_sprite->setScaleY( playerPosition->scale_y );
            }
            else{
                player->setPosition( ccp(playerPosition->position_x * size.width,
                                         playerPosition->position_y * size.height) );
                player->setRotation( -playerPosition->rotation );
                player->setScaleX( playerPosition->scale_x );
                player->setScaleY( playerPosition->scale_y );
            }
        }
        else{
            if (player_sprite) {
                player_sprite->setPosition( ccp(size.width*0.5, size.height*0.5) );
            }
            else{
                player->setPosition( ccp(size.width*0.5, size.height*0.5) );
            }
        }
        if (player_sprite) {
            AXLOGI("PZGCharacterMenu::loadCharacter: Adding player_sprite as child");
            this->addChild( player_sprite, 301 );
        }
        else{
            AXLOGI("PZGCharacterMenu::loadCharacter: Adding player as child");
            this->addChild( player, 301 );
        }
        AXLOGI("PZGCharacterMenu::loadCharacter: Character added to scene");
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Checking player count");
    if (player_n > 1){
        //check for buttons
        Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
        if (menu){
            if (characterId >= player_n-1){
                MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuRightButton );
                if (item) {
                    item->setEnabled( false );
                }
            }
            else{
                MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuRightButton );
                if (item) {
                    item->setEnabled( true );
                }
            }
            
            if (characterId <= 0) {
                MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuLeftButton );
                if (item) {
                    item->setEnabled( false );
                }
            }
            else{
                MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuLeftButton );
                if (item) {
                    item->setEnabled( true );
                }
            }
        }
    }
    else{
        //check for buttons
        Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
        if (menu){
            MenuItemSprite *item;
            item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuRightButton );
            if (item) {
                item->setEnabled( false );
            }
            item = (MenuItemSprite*)menu->getChildByTag( kUIIDCharacterMenuLeftButton );
            if (item) {
                item->setEnabled( false );
            }
        }
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Setting character ID");
    if (!sc) {
        AXLOGE("PZGCharacterMenu::loadCharacter: sc (PZSettingsController) is null");
        return;
    }
    sc->characterId = characterId;
    AXLOGI("PZGCharacterMenu::loadCharacter: Character ID set, checking if unlocked");

    if (sc->isPlayerUnlocked( characterId )) {
        AXLOGI("PZGCharacterMenu::loadCharacter: Player {} is unlocked", characterId);
        Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
        if (menu){
            MenuItemSprite *item;
            item = (MenuItemSprite*)menu->getChildByTag( 3 );
            if (item) {
                item->setOpacity( 0 );
                item->setEnabled( false );
            }
            
            item = (MenuItemSprite*)menu->getChildByTag( 4 );
            if (item) {
                item->setEnabled( true );
            }
        }
        
        if (player_price) {
            player_price->setString("");
        }

    }
    else{
        Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
        if (menu){
            MenuItemSprite *item;
            item = (MenuItemSprite*)menu->getChildByTag( 3 );
            if (item) {
                item->setOpacity( 255 );
                item->setEnabled( true );                
            }

            item = (MenuItemSprite*)menu->getChildByTag( 4 );
            if (item) {
                item->setEnabled( false );
            }

        }
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Checking if character price is 0");
    if (characterInfo->price == 0) {
        AXLOGI("PZGCharacterMenu::loadCharacter: Character is free, calling unlockCallback");
        unlockCallback( NULL );
        AXLOGI("PZGCharacterMenu::loadCharacter: unlockCallback completed");
    }

    AXLOGI("PZGCharacterMenu::loadCharacter: Completed successfully");
}

void PZGCharacterMenu::purchaseComplete(int characterId ){
    PZSettingsController *sc = PZSettingsController::shared();
    sc->unlockPlayer( characterId );
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu){
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 3 );
        item->setOpacity( 0 );
        item->setEnabled( false );
    }
}

void PZGCharacterMenu::update(float dd){
    if (player) {
        player->update( dd );
    }
}


void PZGCharacterMenu::startGameCallback(Object* pSender){
    AXLOGI("PZGCharacterMenu::startGameCallback: Starting");

    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();

    if (buttonClickedSound) {
        buttonClickedSound->playAsSound( false );
    }

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    if (!gsd) {
        AXLOGE("PZGCharacterMenu::startGameCallback: gsd is null");
        return;
    }

    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex( 8 );
        if (soundObj) soundObj->playAsBackgroundMusic();
    }

    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    if (!array || array->count() == 0) {
        AXLOGE("PZGCharacterMenu::startGameCallback: GameplaySettings not found");
        return;
    }

    PZGGameInfoObject* gameInfoObject = (PZGGameInfoObject*)array->objectAtIndex(0);
    if (!gameInfoObject) {
        AXLOGE("PZGCharacterMenu::startGameCallback: gameInfoObject is null");
        return;
    }

    Director *pDirector = Director::getInstance();
    Scene *pScene = nullptr;
    PZGGameFieldScene* bMenu = nullptr;

    AXLOGI("PZGCharacterMenu::startGameCallback: CURRENT GAMEPLAY ID: {}", gameInfoObject->gameTypeId);

    if (gameInfoObject->gameTypeId == 0) {
        AXLOGI("loading: BASIC AVOIDANCE GAME");
        pScene = PZGGamefieldBasicAvoidance::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldBasicAvoidance*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 1) {
        AXLOGI("loading: TILT AVOIDANCE");
        pScene = PZGGamefieldTiltAvoidance::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldTiltAvoidance*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 2) {
        AXLOGI("loading: ROUND SHOOTER");
        pScene = PZGGamefieldRoundShooter::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldRoundShooter*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 3) {
        AXLOGI("loading: SPACE SHOOTER GAME");
        pScene = PZGGamefieldSpaceShooter::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldSpaceShooter*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 4) {
        AXLOGI("loading: HELICOPTER GAME");
        pScene = PZGGamefieldHelicopterGame::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldHelicopterGame*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 5) {
        AXLOGI("loading: SLOW MOVE RUNNER");
        pScene = PZGGamefieldSlowMoveRunner::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldSlowMoveRunner*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 6) {
        AXLOGI("loading: BASIC RUNNER GAME");
        pScene = PZGGamefieldBasicRunner::scene();
        if (!pScene) {
            AXLOGE("PZGCharacterMenu::startGameCallback: BasicRunner scene creation failed (init returned false)");
        }
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldBasicRunner*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 7) {
        AXLOGI("loading: SHOOTING RUNNER GAME");
        pScene = PZGGamefieldShootingRunner::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldShootingRunner*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 8) {
        AXLOGI("loading: SHOOTING HELICOPTER GAME");
        pScene = PZGGamefieldShootingHelicopter::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldShootingHelicopter*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 9) {
        AXLOGI("loading: JUMPING GAME");
        pScene = PZGGamefieldJumpingGame::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldJumpingGame*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 10) {
        AXLOGI("loading: MEGAJUMP GAME");
        pScene = PZGGamefieldMegaJumpGame::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldMegaJumpGame*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }
    else if (gameInfoObject->gameTypeId == 11) {
        AXLOGI("loading: RACING GAME");
        pScene = PZGGamefieldRacingGame::scene();
        if (pScene && !pScene->getChildren().empty()) {
            for (size_t i = 0; i < pScene->getChildren().size(); i++) {
                auto* child = dynamic_cast<PZGGamefieldRacingGame*>(pScene->getChildren().at(i));
                if (child) { bMenu = child; bMenu->load("InterfaceGF"); break; }
            }
        }
    }

    AXLOGI("PZGCharacterMenu::startGameCallback: About to push scene, pScene = {}", pScene != nullptr ? "valid" : "null");
    if (pScene) {
        pDirector->pushScene(pScene);
        AXLOGI("PZGCharacterMenu::startGameCallback: Scene pushed successfully");
    } else {
        AXLOGE("PZGCharacterMenu::startGameCallback: pScene is null, cannot push scene");
    }
}

void PZGCharacterMenu::unlockCallback(Object* pSender){
    
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound( false );
    }

    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    PZSettingsController *sc = PZSettingsController::shared();
    
    __Array* charactersInfoArray= (__Array*)gsd->artResource->objectForKey( "Characters" );
    PZGArtCharacter* characterInfo = ( PZGArtCharacter* )charactersInfoArray->objectAtIndex( player_c );
    
    if (characterInfo) {
        if(characterInfo->enableIAP){
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
            cocos2dx_StoreController::buyMarketItem( characterInfo->storeId->getCString() );
#endif
        }
        else{
            if ( sc->coins >= characterInfo->price ) {
                sc->coins -= characterInfo->price;
                
                sc->unlockPlayer( player_c );
                
                char c[16];
                sprintf(c, "%d", sc->coins);
                coins_label->setString( c );

                if (player_price) {
                    player_price->setString("");
                }

                
                Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
                if (menu){
                    MenuItemSprite *item;
                    item = (MenuItemSprite*)menu->getChildByTag( 3 );
                    if (item) {
                        item->setOpacity( 0 );
                        item->setEnabled( false );
                    }
                    
                    item = (MenuItemSprite*)menu->getChildByTag( 4 );
                    if (item) {
                        item->setEnabled( true );
                    }

                }
            }
        }
    }
}

void PZGCharacterMenu::backCallback(Object* pSender){
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound( false );
    }

    Director *pDirector = Director::getInstance();
    pDirector->popScene();
}

void PZGCharacterMenu::leftCallback(Object* pSender){
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound( false );
    }

    player_c -= 1;
    loadCharacter(player_c);
}

void PZGCharacterMenu::rightCallback(Object* pSender){
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound( false );
    }

    player_c += 1;
    loadCharacter(player_c);
}

void PZGCharacterMenu::upgradeComplete(){
    PZGBaseMenuScene::upgradeComplete();
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 13 ); // UPGRADE BUTTON
        if (item) {
            item->setOpacity( 0 );
            item->setEnabled( false );
        }
    }
}



void PZGCharacterMenu::kidModePurchased(){
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
        item = (MenuItemSprite*)menu->getChildByTag( 11 ); //KIDMODE BUTTON
        if (item) {
            item->setVisible(false);
        }
    }
}



void PZGCharacterMenu::keyBackClicked(){
    backCallback( NULL );
}
