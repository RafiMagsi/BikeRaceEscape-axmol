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
    PZGBaseMenuScene::load( keyName );
    
    PZSettingsController* sc = PZSettingsController::shared();
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    __Array *array = (__Array*) sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex( 0 );
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        
        MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( 4 ); //INDEX 4 is FOR START GAME BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCharacterMenu::startGameCallback, this));
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 5 ); //INDEX 5 is for BACK BUTTON
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCharacterMenu::backCallback, this));
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
            if (sc->removeAds || iapInfo->removeAd_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::menuUpgradeCallback, this));
            }
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 12 ); //COINSHOP
        if (item) {
            if (iapInfo->coinShop_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::coinShopCallback, this));
            }
        }
        item = (MenuItemSprite*)menu->getChildByTag( 11 ); //KIDMODE
        if (item) {
            if (iapInfo->kidMode_enabled && sc->kidMode == false) {
                item->setVisible(true);
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::buyKidModeCallback, this));
            }
            else{
                item->setVisible(false);
            }
        }
        
    }
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    Size size = Director::getInstance()->getWinSize();
    
    PZGArtInterface *coinLabelPosition = getItemByName("GUI_CoinsLabelLocation", keyName);
    if (coinLabelPosition) {
        coins_label = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
        coins_label->setScaleX( coinLabelPosition->scale_x);
        coins_label->setScaleY( coinLabelPosition->scale_y);
        coins_label->setPosition( coinLabelPosition->getPosition() );
        coins_label->setRotation( -coinLabelPosition->rotation );
        coins_label->setAlignment( TextHAlignment::LEFT );
        coins_label->setAnchorPoint( ccp(0, 0.5) );
        this->addChild( coins_label, 1000 );
    }
    
    
    PZGArtInterface *playerNameLabelPosition = getItemByName("GUI_CharacterNameLabelLocation", keyName);

    if (playerNameLabelPosition) {
        player_name = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
        player_name->setScaleX( playerNameLabelPosition->scale_x);
        player_name->setScaleY( playerNameLabelPosition->scale_y);
        player_name->setPosition( playerNameLabelPosition->getPosition() );
        player_name->setRotation( -playerNameLabelPosition->rotation );
        player_name->setAlignment( TextHAlignment::CENTER );
        this->addChild( player_name, 1000 );        
    }
    
    PZGArtInterface *playerPriceLabelPosition = getItemByName("GUI_CharacterPriceLabelLocation", keyName);
    
    if (playerPriceLabelPosition) {
        player_price = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
        player_price->setScaleX( playerPriceLabelPosition->scale_x);
        player_price->setScaleY( playerPriceLabelPosition->scale_y);
        player_price->setPosition( playerPriceLabelPosition->getPosition() );
        player_price->setRotation( -playerPriceLabelPosition->rotation );
        player_price->setAlignment( TextHAlignment::CENTER );
        this->addChild( player_price, 1000 );
    }

    playerPosition = getItemByName("GUI_CharacterLocation", keyName);

    loadCharacter( player_c );
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
    PZSettingsController *sc = PZSettingsController::shared();
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    Size size = Director::getInstance()->getWinSize();
    
    if (player != NULL){
        this->removeChild(player, true);
        player = NULL;
    }
    if (player_sprite != NULL) {
        this->removeChild(player_sprite, true);
        player_sprite = NULL;
    }

    __Array* charactersInfoArray = (__Array*)gsd->artResource->objectForKey( "Characters" );
    if (charactersInfoArray == NULL) {
        return;
    }
    PZGArtCharacter* characterInfo = ( PZGArtCharacter* )charactersInfoArray->objectAtIndex( characterId );
    if (characterInfo == NULL) {
        return;
    }
    if (characterInfo->iconArtObj) {
        player_sprite = characterInfo->iconArtObj->getResource();
    }
    
    if ( player_sprite == NULL) {
        player = PZGCharacter::create( characterId, NULL, false );
    }
    
    if (player || player_sprite) {
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
            this->addChild( player_sprite, 301 );
        }
        else{
            this->addChild( player, 301 );
        }

    }
    
    
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

    sc->characterId = characterId;

    if (sc->isPlayerUnlocked( characterId )) {
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
    
    if (characterInfo->price == 0) {
        unlockCallback( NULL );
    }
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
