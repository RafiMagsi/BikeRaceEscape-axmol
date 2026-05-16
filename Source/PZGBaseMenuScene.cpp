//
//  BaseMenuScene.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/13/12.
//
//

#include "PZGBaseMenuScene.h"
#include "PZSettingsController.h"
#include "PZGCoinShopScene.h"
#include "constants.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "cocos2dx_StoreController.h"
    #include "Neocortex.h"
#endif

USING_NS_AX;

ax::Scene* PZGBaseMenuScene::scene( ){
    
	ax::Scene *scene = ax::Scene::create();
 	PZGBaseMenuScene *layer = PZGBaseMenuScene::create();
	scene->addChild(layer);
	return scene;
    
}


bool PZGBaseMenuScene::init(){
	if ( !Layer::init() )	{
		return false;
	}
    
    
    return true;
}


PZGArtInterface* PZGBaseMenuScene::getItemByName( const char* name, const char* interfaceKey){

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

void PZGBaseMenuScene::load(const char* keyName){
    
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
            
            sprite->setTag( infoObj->index );
            sprite->setPosition( infoObj->getPosition() );
            
            sprite->setRotation( -infoObj->rotation );
            sprite->setScaleX( infoObj->scale_x );
            sprite->setScaleY( infoObj->scale_y );
            
//            //fill up background images
//            if (infoObj->position_lock == true) {
//                float scale = 1.0;
//                Size size = Director::getInstance()->getWinSize();
//                float winAr = size.width/size.height;
//                
//                Size splashSize = sprite->getContentSize();
//                float splashAr = splashSize.width/splashSize.height;
//                
//                if (winAr > splashAr) {
//                    scale = size.width/splashSize.width;
//                }
//                else{
//                    scale = size.height/splashSize.height;
//                }
//                sprite->setScale( scale );
//            }
            
            this->addChild(sprite, 300);
        }
    }
    
    if (pMenu->!getChildren().empty()) {
        this->addChild(pMenu, 302);
    }
    
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        buttonClickedSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDButtonPressed );
        buttonClickedSound->preloadSound();
    }
}



#pragma mark IAP

void PZGBaseMenuScene::menuUpgradeCallback(Ref* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }
    
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    if (array) {
        PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
        if (iapInfo->removeAd_id) {
            cocos2dx_StoreController::buyMarketItem(iapInfo->removeAd_id->getCString());
        }
    }
    
#endif
    
}
void PZGBaseMenuScene::upgradeComplete(){
    PZSettingsController* sc = PZSettingsController::shared();
    sc->removeAds = true;
    sc->save();
    
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::Neocortex::shared()->synchronized = false;
    PZ::Neocortex::shared()->removeBanner();
#endif
}

void PZGBaseMenuScene::coinShopCallback(Ref* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }
    
    Director *pDirector = Director::getInstance();
    Scene *pScene = PZGCoinShopScene::scene();
    PZGCoinShopScene *bMenu = (PZGCoinShopScene *)pScene->getChildren().at( 0 );
    bMenu->load( "InterfaceCoinShop" );
    pDirector->pushScene(pScene);
    
}

void PZGBaseMenuScene::offKidModeCallback(Ref* pSender){
    
    PZSettingsController *sc = PZSettingsController::shared();
    sc->kidMode = false;
    sc->save();
}

void PZGBaseMenuScene::buyKidModeCallback(Ref* pSender){
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }
    
    PZSettingsController *sc = PZSettingsController::shared();
    if (sc->kidModePurchased) {
        sc->kidMode = true;
        sc->save();
        kidModePurchased();
        return;
    }
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    ax::__Array* array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
    
    if (iapInfo->kidMode_id) {
        cocos2dx_StoreController::buyMarketItem(iapInfo->kidMode_id->getCString());
    }
    
#endif
}

void PZGBaseMenuScene::kidModePurchased(){

    
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
    }
}
