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
    
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();

    if (buttonClickedSound) {
        buttonClickedSound->playAsSound( false );
    }

    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    //Sound
    
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        PZGSoundData *soundObj =  (PZGSoundData*)sounds->objectAtIndex( 8 );
        soundObj->playAsBackgroundMusic();        
    }
    
    
    /////////////////////////////////////////////
    /// loading proper gameplay
    
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameInfoObject* gameInfoObject = (PZGGameInfoObject*)array->objectAtIndex( 0 );
    
    Director *pDirector = Director::getInstance();
    Scene *pScene = NULL;
    PZGGameFieldScene* bMenu = NULL;
    
    printf("CURRENT GAMEPLAY ID: %d\n", gameInfoObject->gameTypeId);
    
    if (gameInfoObject->gameTypeId == 0){

        printf("loading: BASIC AVOIDANCE GAME\n");
        
        pScene = PZGGamefieldBasicAvoidance::scene();
        bMenu = (PZGGamefieldBasicAvoidance *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );            
    }
    else if (gameInfoObject->gameTypeId == 1){
        printf("loading: TILT AVOIDANCE\n");
        
        pScene = PZGGamefieldTiltAvoidance::scene();
        bMenu = (PZGGamefieldTiltAvoidance *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    else if (gameInfoObject->gameTypeId == 2){
        printf("loading: ROUND SHOOOTER\n");
        
        pScene = PZGGamefieldRoundShooter::scene();
        bMenu = (PZGGamefieldRoundShooter *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    else if (gameInfoObject->gameTypeId == 3){
        printf("loading: SPACE SHOOTER GAME\n");
        
        pScene = PZGGamefieldSpaceShooter::scene();
        bMenu = (PZGGamefieldSpaceShooter *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    else if (gameInfoObject->gameTypeId == 5){
        printf("loading: SLOW MOVE RUNNER\n");
        
        pScene = PZGGamefieldSlowMoveRunner::scene();
        bMenu = (PZGGamefieldSlowMoveRunner *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    else if (gameInfoObject->gameTypeId == 4){
        printf("loading: HELICOPTER GAME\n");
        
        pScene = PZGGamefieldHelicopterGame::scene();
        bMenu = (PZGGamefieldHelicopterGame *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    else if (gameInfoObject->gameTypeId == 6){
        printf("loading: BASIC RUNNER GAME\n");
        
        pScene = PZGGamefieldBasicRunner::scene();
        bMenu = (PZGGamefieldBasicRunner *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    else if (gameInfoObject->gameTypeId == 7){
        printf("loading: SHOOTING RUNNER GAME\n");
        
        pScene = PZGGamefieldShootingRunner::scene();
        bMenu = (PZGGamefieldShootingRunner *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    else if (gameInfoObject->gameTypeId == 8){
        printf("loading: SHOOTING HELICOPTER GAME\n");
        
        pScene = PZGGamefieldShootingHelicopter::scene();
        bMenu = (PZGGamefieldShootingHelicopter *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }else if (gameInfoObject->gameTypeId == 9){
        printf("loading: JUMPING GAME\n");
        
        pScene = PZGGamefieldJumpingGame::scene();
        bMenu = (PZGGamefieldJumpingGame *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }else if (gameInfoObject->gameTypeId == 10){
        printf("loading: MEGAJUMP GAME\n");
        
        pScene = PZGGamefieldMegaJumpGame::scene();
        bMenu = (PZGGamefieldMegaJumpGame *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }else if (gameInfoObject->gameTypeId == 11){
        printf("loading: Racing GAME\n");
        
        pScene = PZGGamefieldRacingGame::scene();
        bMenu = (PZGGamefieldRacingGame *)pScene->getChildren().at( 0 );
        bMenu->load( "InterfaceGF" );
    }
    
    pDirector->pushScene( pScene );

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
