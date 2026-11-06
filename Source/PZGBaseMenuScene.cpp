//
//  BaseMenuScene.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/13/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGBaseMenuScene.h"
#include "Features/PZGameConfig.h"
#include "Features/PZLayout.h"
#include "PZSettingsController.h"
#include "PZGCoinShopScene.h"
#include "Features/PZPopups.h"
#include "Features/PZUI.h"
#include "constants.h"
#include <cstring>
#include "Ads/AdsController.h"

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


// Re-arranges the plist-driven buttons of a screen. Runs before the entrance animation is built (the stagger remembers
// each item's position as its "home"), so the new spots are the ones the buttons animate to.
static void arrangeInterface(const char* key, ax::Node* layer, ax::Menu* menu) {
    if (!key || !menu) return;
    const ax::Size win = ax::Director::getInstance()->getWinSize();
    const ax::Rect safe = PZLayout::safeRect();
    auto item = [menu](int tag) { return dynamic_cast<ax::MenuItem*>(menu->getChildByTag(tag)); };

    if (std::strcmp(key, "InterfaceMM") == 0) {
        // Restore / kids bike / coin store stack down the left edge; the logo takes the middle of the screen.
        const float S = PZLayout::uiScale();
        const float colX = safe.getMinX() + 108.f * S;
        const float ys[3] = {win.height * 0.80f, win.height * 0.595f, win.height * 0.39f};
        const int tags[3][2] = {{kUIIDMainMenuRestoreIAP, -1}, {kUIIDMainMenuKidModeIAP, kUIIDMainMenuKidModeOff}, {kUIIDMainMenuCoinShop, -1}};
        for (int i = 0; i < 3; ++i) {
            for (int t : tags[i]) {
                if (t < 0) continue;
                if (auto* it = item(t)) {
                    it->setScale(0.92f * S);
                    it->setPosition(ax::Vec2(colX, ys[i]));
                }
            }
        }
        // PLAY is centred under the XP plate and pills (same column), and sits a little lower.
        if (auto* play = item(3)) {
            play->setPosition(ax::Vec2(safe.getMaxX() - 12.f - 145.f * S, play->getPositionY() - 26.f));
            play->setScale(play->getScale() * S);
        }
        if (auto* gc = item(kUIIDMainMenuGameCenter)) gc->setScale(gc->getScale() * S);
        if (auto* logo = layer ? layer->getChildByTag(1) : nullptr) {
            logo->setPosition(ax::Vec2(win.width * (S < 0.995f ? 0.38f : 0.45f), win.height * 0.69f));
            logo->setScale(logo->getScale() * std::min(1.f, S * 1.05f));
        }
    } else if (std::strcmp(key, "InterfaceGF") == 0) {
        // Gameplay HUD art (pause button, coin icon) shrinks on narrow screens; done here so the entrance animation
        // records the final scale.
        const float S = PZLayout::uiScale();
        if (S < 0.995f) {
            if (auto* pause = item(kUIIDGameFieldPauseButton)) pause->setScale(pause->getScale() * S);
            if (auto* coin = layer ? layer->getChildByTag(1) : nullptr) coin->setScale(coin->getScale() * S);
        }
    } else if (std::strcmp(key, "InterfaceCS") == 0) {
        // BACK sits on the same row as PLAY (it used to hang lower than the big button).
        // Narrow screens: the extra buttons shrink (the COIN STORE art was far too big on iPad).
        const float S = PZLayout::uiScale();
        if (S < 0.995f) {
            for (int t : {11, 12, 13}) {
                if (auto* it = item(t)) it->setScale(it->getScale() * S);
            }
        }
        auto* play = item(4);
        if (play) {
            for (int t : {5}) {
                if (auto* it = item(t)) it->setPositionY(play->getPositionY());
            }
        }
    }
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
            // Keep every interface button above the banner ad and inside the device safe area.
            menuItem->setPosition( ccp(infoObj->getPosition().x, PZLayout::mapY(infoObj->getPosition().y)) );
            menuItem->setRotation( -infoObj->rotation );
            menuItem->setScaleX( infoObj->scale_x );
            menuItem->setScaleY( infoObj->scale_y );
            // One standard size for the round navigation buttons (BACK / RETRY / CONTINUE) on every screen.
            if (infoObj->subkey) {
                const std::string sk = infoObj->subkey->getCString();
                if (sk == "GUI_BackButton" || sk == "GUI_RestartButton" || sk == "GUI_ContinueButton") {
                    const Size cs = sprite->getContentSize();
                    const float natural = std::max(cs.width, cs.height);
                    if (natural > 1.f) menuItem->setScale(PZCfg::kStdRoundButtonPx / natural);
                }
            }
            menuItem->setTag( infoObj->index );

            pMenu->addChild(menuItem);
            PZLayout::keepInside(menuItem);
            AXLOGI("    MenuItem added, menu now has {} children", pMenu->getChildren().size());
        }
        else if (infoObj->type && infoObj->type->compare("CCSprite") == 0) {
            Sprite *sprite = infoObj->getResource();
            if (sprite == NULL) {
                continue;
            }
            
            sprite->setTag( infoObj->index );
            sprite->setPosition( infoObj->position_lock ? infoObj->getPosition()
                                                        : ccp(infoObj->getPosition().x, PZLayout::mapY(infoObj->getPosition().y)) );
            
            sprite->setRotation( -infoObj->rotation );
            sprite->setScaleX( infoObj->scale_x );
            sprite->setScaleY( infoObj->scale_y );

            // Fill the screen with position-locked background art. The menu backdrop is 1370x768 while a
            // 20:9 phone exposes 1422x640 design units with FIXED_HEIGHT, which left black bars on both
            // sides. Scale uniformly to cover the visible area on any device aspect ratio.
            if (infoObj->position_lock) {
                const Size winSize = Director::getInstance()->getWinSize();
                const Size spriteSize = sprite->getContentSize();
                if (spriteSize.width > 0.0f && spriteSize.height > 0.0f) {
                    const float winAr = winSize.width / winSize.height;
                    const float spriteAr = spriteSize.width / spriteSize.height;
                    const float scale = (winAr > spriteAr)
                        ? winSize.width / spriteSize.width
                        : winSize.height / spriteSize.height;
                    sprite->setScale(scale);
                }
            }

            this->addChild(sprite, 300);
        }
    }
    
    arrangeInterface(keyName, this, pMenu);
    if (!pMenu->getChildren().empty()) {
        this->addChild(pMenu, 302);
        AXLOGI("PZGBaseMenuScene::load - Added menu with {} items", pMenu->getChildren().size());
        // Buttons arrive one after the other instead of all appearing at once.
        PZUI::stagger(pMenu, 0.055f, 0.12f);
    } else {
        AXLOGI("PZGBaseMenuScene::load - Menu is empty, no items added");
    }

    // The main menu fades in from onEnter; the game field has its own cinematic intro.
    // Avoid stacking a second black overlay on either screen.
    if (!keyName || (std::strcmp(keyName, "InterfaceGF") != 0 && std::strcmp(keyName, "InterfaceMM") != 0)) {
        PZUI::fadeInScreen(this);
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
    // Keep AdsController in sync with the "Remove Ads" purchase.
    PZ::AdsController::shared()->setAdsEnabled(false);
#endif
    
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::Neocortex::shared()->synchronized = false;
    PZ::Neocortex::shared()->removeBanner();
#endif
}

void PZGBaseMenuScene::coinShopCallback(Object* pSender) {
    if(buttonClickedSound){
        buttonClickedSound->playAsSound( false );
    }

    // The coin store is the SHOP overlay's COINS tab now: same six packs, plus the rewarded-video top-up, in the
    // code-drawn style the rest of the meta UI uses (the old three-pack InterfaceCoinShop scene is retired).
    PZPopups::showShop(this, [this]() { PZPopups::refreshMenuWidgets(this, false); }, true);
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
