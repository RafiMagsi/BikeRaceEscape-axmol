//
//  PZGCoinShopScene.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/9/13.
//
//

#include "PZGCoinShopScene.h"
#include "SharedData/PZGSharedData.h"
#include "PZSettingsController.h"
#include "constants.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "StoreBridge/cocos2dx_StoreController.h"
#endif

USING_NS_AX;

Scene* PZGCoinShopScene::scene()
{
	Scene *scene = Scene::create();
	PZGCoinShopScene *layer = PZGCoinShopScene::create();
	scene->addChild(layer);
	return scene;
}

bool PZGCoinShopScene::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    this->setKeypadEnabled(true);
#endif
    
	return true;
}


void PZGCoinShopScene::load(const char* keyName){
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* uiItems = (__Array*)gsd->artResource->objectForKey( keyName );
    
    if (uiItems == NULL) {
        return;
    }
    
    Size size = Director::getInstance()->getWinSize();
    
    Menu* pMenu = Menu::create();
    pMenu->setAnchorPoint( ccp(0,0) );
    pMenu->setPosition( ccp(0,0) );
    pMenu->setTag( kBaseMenuItemTag );
    
    for (int i=0; i < uiItems->count(); i++) {
        PZGArtInterface *infoObj = (PZGArtInterface*)uiItems->objectAtIndex( i );
        
        if (infoObj->type->compare("MenuItemSprite") == 0) {
            MenuItemSprite* menuItem;
            
            Sprite *sprite = infoObj->getResource();
            if (sprite == NULL) {
                continue;
            }
            
            Sprite *spriteClick = infoObj->getResource();
            spriteClick->setColor( ccc3(128, 128, 128) );
            
            Sprite *spriteDisabled = infoObj->getResource();
            spriteDisabled->setColor( ccc3(60, 60, 60) );
            menuItem = MenuItemSprite::create(sprite, spriteClick, spriteDisabled);
            menuItem->setAnchorPoint( ccp(0.5, 0.5) );
            menuItem->setPosition( infoObj->getPosition() );
            menuItem->setRotation( -infoObj->rotation );
            menuItem->setScaleX( infoObj->scale_x );
            menuItem->setScaleY( infoObj->scale_y );
            menuItem->setTag( infoObj->index );
            
            pMenu->addChild(menuItem);
        }
        else if(infoObj->type->compare("Sprite") == 0){
            Sprite *sprite = infoObj->getResource();
            if (sprite == NULL) {
                continue;
            }
            sprite->setAnchorPoint( ccp(0.5, 0.5) );
            sprite->setTag( infoObj->index );
            sprite->setPosition( infoObj->getPosition() );
            
            sprite->setRotation( -infoObj->rotation );
            sprite->setScaleX( infoObj->scale_x );
            sprite->setScaleY( infoObj->scale_y );
            
            this->addChild(sprite, 300);
        }
    }
    
    if (pMenu->!getChildren().empty()) {
        this->addChild(pMenu, 300);
    }
    
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        buttonClickedSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDButtonPressed );
    }
    
   
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        
        PZGArtInterface *backButtonInfo = (PZGArtInterface*)getItemByName( "GUI_BackButton", keyName );
        MenuItemSprite *item = (MenuItemSprite*)menu->getChildByTag( backButtonInfo->index );
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCoinShopScene::menuBackCallback, this));
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 2 );
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCoinShopScene::buyCoinPack1Callback, this));
        }

        item = (MenuItemSprite*)menu->getChildByTag( 3 );
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCoinShopScene::buyCoinPack2Callback, this));
        }

        item = (MenuItemSprite*)menu->getChildByTag( 4 );
        if (item) {
            item->setCallback(AX_CALLBACK_1(PZGCoinShopScene::buyCoinPack3Callback, this));
        }
    }
}

PZGArtInterface* PZGCoinShopScene::getItemByName( const char* name, const char* interfaceKey){
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* uiItems = (__Array*)gsd->artResource->objectForKey( interfaceKey );
    
    for (int i=0; i < uiItems->count(); i++) {
        PZGArtInterface *infoObj = (PZGArtInterface*)uiItems->objectAtIndex( i );
        if ( infoObj->subkey->compare( name ) == 0) {
            return infoObj;
        }
    }
    
    return NULL;
}


void PZGCoinShopScene::buyCoinPack1Callback(Ref* pSender){
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound(false);
    }
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)

    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
    
    if (iapInfo->coinShop1_id) {
        cocos2dx_StoreController::buyMarketItem(iapInfo->coinShop1_id->getCString());
    }

#endif
}

void PZGCoinShopScene::buyCoinPack2Callback(Ref* pSender){
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound(false);
    }
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);

    if (iapInfo->coinShop2_id) {
        cocos2dx_StoreController::buyMarketItem(iapInfo->coinShop2_id->getCString());
    }
    
#endif
}

void PZGCoinShopScene::buyCoinPack3Callback(Ref* pSender){
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound(false);
    }

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
    
    if (iapInfo->coinShop3_id) {
        cocos2dx_StoreController::buyMarketItem(iapInfo->coinShop3_id->getCString());
    }
    
#endif
}

void PZGCoinShopScene::menuBackCallback(Ref* pSender){
    if (buttonClickedSound) {
        buttonClickedSound->playAsSound(false);
    }
    
    Director *pDirector = Director::getInstance();
    pDirector->popScene();
}

void PZGCoinShopScene::keyBackClicked(){
    menuBackCallback( NULL );
}
