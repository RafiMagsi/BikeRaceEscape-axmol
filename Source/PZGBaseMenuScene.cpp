//
//  BaseMenuScene.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/13/12.
//
//

#include "PZLegacyCompat.h"
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

    if (!name || !interfaceKey) {
        AXLOGW("PZGBaseMenuScene::getItemByName: name or interfaceKey is null");
        return nullptr;
    }

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    if (!gsd || !gsd->artResource) {
        AXLOGW("PZGBaseMenuScene::getItemByName: gsd or artResource is null");
        return nullptr;
    }

    __Array* uiItems = (__Array*)gsd->artResource->objectForKey( interfaceKey );
    if (!uiItems) {
        AXLOGW("PZGBaseMenuScene::getItemByName: No UI items found for key: {}", interfaceKey);
        return nullptr;
    }

    for (int i=0; i < uiItems->count(); i++) {
        PZGArtInterface *infoObj = (PZGArtInterface*)uiItems->objectAtIndex( i );
        if (!infoObj || !infoObj->subkey) {
            AXLOGW("PZGBaseMenuScene::getItemByName: Item {} has null infoObj or subkey", i);
            continue;
        }
        if ( infoObj->subkey->compare( name ) == 0) {
            AXLOGI("PZGBaseMenuScene::getItemByName: Found item '{}' in key '{}'", name, interfaceKey);
            return infoObj;
        }
    }

    AXLOGW("PZGBaseMenuScene::getItemByName: Item '{}' not found in key '{}'", name, interfaceKey);
    return nullptr;
}

void PZGBaseMenuScene::load(const char* keyName){
    AXLOGI("PZGBaseMenuScene::load begin - keyName: {}", keyName ? keyName : "(null)");

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* uiItems = (__Array*)gsd->artResource->objectForKey( keyName );

    if (uiItems == NULL) {
        AXLOGE("PZGBaseMenuScene::load - No UI items found for key: {}", keyName ? keyName : "(null)");
        AXLOGI("Available artResource: {} entries in resource dictionary", gsd->artResource ? 1 : 0);
        AXLOGI("  This usually means the key name doesn't match plist category or items weren't parsed correctly");
        AXLOGI("  Expected key: '{}'", keyName ? keyName : "(null)");
        AXLOGI("  Check: CompleteGameInfo.plist structure, interface key values, and PZGSharedData parsing");
        return;
    }
    AXLOGI("PZGBaseMenuScene::load - Found {} UI items for key: {}", uiItems->count(), keyName);

    Size size = Director::getInstance()->getWinSize();

    Menu* pMenu = Menu::create();
    pMenu->setAnchorPoint( ccp(0,0) );
    pMenu->setPosition( ccp(0,0) );
    pMenu->setTag( kBaseMenuItemTag );

    for (int i=0; i < uiItems->count(); i++) {
        PZGArtInterface *infoObj = (PZGArtInterface*)uiItems->objectAtIndex( i );

        if (!infoObj) {
            AXLOGW("  Item {} is null", i);
            continue;
        }

        const char* itemType = infoObj->type ? infoObj->type->getCString() : "(null)";
        AXLOGI("  Item {}: type='{}', subkey='{}'", i, itemType,
            infoObj->subkey ? infoObj->subkey->getCString() : "(null)");

        if (infoObj->type && infoObj->type->compare("CCMenuItemSprite") == 0) {
            MenuItemSprite* menuItem;

            Sprite *sprite = infoObj->getResource();
            if (sprite == NULL) {
                AXLOGW("    MenuItemSprite skipped: getResource() returned NULL");
                continue;
            }

            AXLOGI("    Creating MenuItemSprite for subkey: {}", infoObj->subkey->getCString());
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
            AXLOGI("    MenuItem added, menu now has {} children", pMenu->getChildren().size());
        }
        else if (infoObj->type && infoObj->type->compare("CCSprite") == 0) {
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
    
    if (!pMenu->getChildren().empty()) {
        this->addChild(pMenu, 302);
        AXLOGI("PZGBaseMenuScene::load - Added menu with {} items", pMenu->getChildren().size());
    } else {
        AXLOGI("PZGBaseMenuScene::load - Menu is empty, no items added");
    }

    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        auto* obj = sounds->objectAtIndex(kSoundIDButtonPressed);
        buttonClickedSound = dynamic_cast<PZGSoundData*>(obj);
        if (buttonClickedSound) {
            buttonClickedSound->preloadSound();
            AXLOGI("PZGBaseMenuScene::load - Sound loaded (index={})", kSoundIDButtonPressed);
        } else {
            AXLOGW("PZGBaseMenuScene::load - Button sound missing at index={} (sounds count={})",
                   kSoundIDButtonPressed, sounds->count());
        }
    }
    AXLOGI("PZGBaseMenuScene::load completed");
}



#pragma mark IAP

void PZGBaseMenuScene::menuUpgradeCallback(Object* pSender) {
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

void PZGBaseMenuScene::coinShopCallback(Object* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }
    
    auto* pDirector = Director::getInstance();

    // Avoid assuming scene()->children[0] exists. Build explicitly so failures are handled.
    auto* pScene = Scene::create();
    auto* layer = PZGCoinShopScene::create();
    if (!pScene || !layer) {
        AXLOGE("coinShopCallback: failed to create coin shop (scene={}, layer={})", (void*)pScene, (void*)layer);
        return;
    }
    pScene->addChild(layer);
    layer->load("InterfaceCoinShop");
    pDirector->pushScene(pScene);
    
}

void PZGBaseMenuScene::offKidModeCallback(Object* pSender){
    
    PZSettingsController *sc = PZSettingsController::shared();
    sc->kidMode = false;
    sc->save();
}

void PZGBaseMenuScene::buyKidModeCallback(Object* pSender){
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
