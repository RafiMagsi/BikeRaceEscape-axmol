/*
 * Copyright (C) 2012 Soomla Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PZLegacyCompat.h"
#include "EventHandler.h"
#include "SharedData/PZGSharedData.h"
#include "cocos2dx_StoreInventory.h"
#include "PZGCharacterMenu.h"
#include "PZGMainMenuScene.h"
#include "Ads/AdsController.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "PZGameCenter.h"
#endif

namespace {

// The running scene's menu layer (it owns the UI refresh for a purchase), or null when the running scene has none.
PZGBaseMenuScene* findRunningMenuScene() {
    ax::Director* director = ax::Director::getInstance();
    ax::Scene* scene = director ? director->getRunningScene() : nullptr;
    if (!scene) return nullptr;
    for (auto* c : scene->getChildren()) {
        if (auto* m = dynamic_cast<PZGBaseMenuScene*>(c)) return m;
    }
    return nullptr;
}

// "Remove Ads" bought or restored. PMP_RemoveAds is only ever set here, and it must be saved even when no menu
// layer is running (e.g. a restore that completes during gameplay), otherwise the purchase is lost.
void applyRemoveAdsPurchased() {
    if (auto* menu = findRunningMenuScene()) {
        menu->upgradeComplete(); // sets removeAds, saves, disables ads and removes the banner
        return;
    }
    AXLOGW("[IAP] PZGBaseMenuScene not found; applying RemoveAds via settings only");
    if (PZSettingsController* sc = PZSettingsController::shared()) {
        sc->removeAds = true;
        sc->save();
    }
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::AdsController::shared()->setAdsEnabled(false);
#endif
}

// "Kid Mode" bought or restored. PMP_KidMode / PMP_KidModePurchased are only set to true here (or by re-enabling an
// already purchased Kid Mode), and are saved even when no menu layer is running.
void applyKidModePurchased() {
    if (auto* menu = findRunningMenuScene()) {
        menu->kidModePurchased(); // sets kidMode + kidModePurchased, saves, refreshes the buttons
        return;
    }
    AXLOGW("[IAP] PZGBaseMenuScene not found; applying KidMode via settings only");
    if (PZSettingsController* sc = PZSettingsController::shared()) {
        sc->kidMode = true;
        sc->kidModePurchased = true;
        sc->save();
    }
}

} // namespace

void EventHandler::marketPurchase(string& productId) {
	printf("Market Purcahse %s\n", productId.c_str());
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    PZ::GameCenter::hideProcessingView();
#endif
    
  //  check for character IAP
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    if (!sd || !sd->artResource) {
        AXLOGE("[IAP] marketPurchase: SharedData/artResource missing");
        return;
    }

    ax::__Array* array = (ax::__Array*)sd->artResource->objectForKey("Characters");
    if (!array) {
        AXLOGW("[IAP] marketPurchase: Characters array missing");
        return;
    }
    
    for (int i = 0; i < array->count(); i++) {
        PZGArtCharacter *character = (PZGArtCharacter*) array->objectAtIndex( i );
        if (!character || !character->storeId) continue;
        if (strcmp(character->storeId->getCString(), productId.c_str()) == 0) {
            
            ax::Director* director = ax::Director::getInstance();
            ax::Scene* scene = director->getRunningScene();
            if (!scene) {
                AXLOGW("[IAP] marketPurchase: no running scene for character purchase");
                return;
            }

            // Never assume child[0] is the menu; search safely.
            PZGCharacterMenu* menu = nullptr;
            const auto& children = scene->getChildren();
            for (auto* c : children) {
                if (auto* m = dynamic_cast<PZGCharacterMenu*>(c)) {
                    menu = m;
                    break;
                }
            }
            if (!menu) {
                AXLOGW("[IAP] marketPurchase: PZGCharacterMenu not found in running scene");
                return;
            }

            menu->purchaseComplete(i);
            return;
        }
    }
    
    //check for RemoveAd IAP
    if (!sd->gameInfoResource) {
        AXLOGW("[IAP] marketPurchase: gameInfoResource missing");
        return;
    }
    array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (array && array->count() > 0) ? (PZGGameInfoIAP*)array->objectAtIndex(0) : nullptr;
    if (!iapInfo) {
        AXLOGW("[IAP] marketPurchase: IAPSettings missing/empty");
        return;
    }

    const char* removeAdId = (iapInfo->removeAd_id ? iapInfo->removeAd_id->getCString() : "");
    if (removeAdId[0] != '\0' && strcmp(removeAdId, productId.c_str()) == 0) {
        applyRemoveAdsPurchased();
        return;
    }
    
    //check for KidMode
    const char* kidModeId = (iapInfo->kidMode_id ? iapInfo->kidMode_id->getCString() : "");
    if (kidModeId[0] != '\0' && strcmp(kidModeId, productId.c_str()) == 0) {
        applyKidModePurchased();
        return;
    }

    //check for CoinPacks (all six packs from IAPSettings)
    PZSettingsController *sc = PZSettingsController::shared();
    ax::__String* packIds[6] = {iapInfo->coinShop1_id, iapInfo->coinShop2_id, iapInfo->coinShop3_id,
                                iapInfo->coinShop4_id, iapInfo->coinShop5_id, iapInfo->coinShop6_id};
    const int packValues[6] = {iapInfo->coinShop1_value, iapInfo->coinShop2_value, iapInfo->coinShop3_value,
                               iapInfo->coinShop4_value, iapInfo->coinShop5_value, iapInfo->coinShop6_value};
    for (int i = 0; i < 6; ++i) {
        const char* packId = (packIds[i] ? packIds[i]->getCString() : "");
        if (packId[0] != '\0' && strcmp(packId, productId.c_str()) == 0) {
            sc->coins += packValues[i];
            sc->save();
            AXLOGI("[IAP] coin pack {} credited with {} coins", i + 1, packValues[i]);
            break;
        }
    }
//    else
//    PZSettingsController *sc = PZSettingsController::shared();
//        if (strcmp("com.chasenetinc.BikeRaceEscape.kidmode", productId.c_str()) == 0 ) {
//            sc->unlockAllPlayers();
//            
//            ax::Director* director = ax::Director::getInstance();
//            ax::Scene* scene = director->getRunningScene();
//            PZGBaseMenuScene *menu = (PZGBaseMenuScene*)scene->getChildren().at( 0 );
//            menu->upgradeComplete();
//            
//        }
}
void EventHandler::transactionRestored(string& productId){
    printf("Transaction Restored: %s\n", productId.c_str());
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    PZ::GameCenter::hideProcessingView();
#endif
    
    //CHARACETRS
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    if (!sd || !sd->artResource || !sd->gameInfoResource) {
        AXLOGE("[IAP] transactionRestored: SharedData resources missing");
        return;
    }
    ax::__Array* array = (ax::__Array*)sd->artResource->objectForKey("Characters");
    if (!array) return;
    for (int i=0; i < array->count(); i++){
        PZGArtCharacter *character = (PZGArtCharacter*) array->objectAtIndex( i );
        if (!character || !character->storeId) continue;
        if (strcmp(character->storeId->getCString(), productId.c_str()) == 0) {
            
            PZSettingsController *sc = PZSettingsController::shared();
            sc->unlockPlayer( i );
            return;
        }
    }
    
    //check for RemoveAd IAP
    array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (array && array->count() > 0) ? (PZGGameInfoIAP*)array->objectAtIndex(0) : nullptr;
    if (!iapInfo) return;
    const char* removeAdId = (iapInfo->removeAd_id ? iapInfo->removeAd_id->getCString() : "");
    if ( removeAdId[0] != '\0' && strcmp(removeAdId, productId.c_str())==0 ) {
        applyRemoveAdsPurchased();
        return;
    }
    
    //check for KidMode
    const char* kidModeId = (iapInfo->kidMode_id ? iapInfo->kidMode_id->getCString() : "");
    if ( kidModeId[0] != '\0' && strcmp( kidModeId, productId.c_str() )==0 ) {
        applyKidModePurchased();
        return;
    }
}
void EventHandler::marketRefund(string& productId) {
	
}

void EventHandler::virtualGoodPurchased(string& itemId) {

}

void EventHandler::virtualGoodEquipped(string& itemId) {
	
}

void EventHandler::virtualGoodUnequipped(string& itemId) {

}

void EventHandler::billingSupported() {

}

void EventHandler::billingNotSupported() {

}

void EventHandler::marketPurchaseProcessStarted(string& productId) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        PZ::GameCenter::showProcessingView();
#endif
}

void EventHandler::goodsPurchaseProcessStarted() {

}

void EventHandler::closingStore() {

}
void EventHandler::paymentCancelled(){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    PZ::GameCenter::hideProcessingView();
#endif
}
void EventHandler::unexpectedErrorInStore(string& message) {
    // if (message != NULL)
    // {

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    PZ::GameCenter::hideProcessingView();
    PZ::GameCenter::showErrorView( message.c_str() );
#endif

    // }
}

void EventHandler::openingStore() {

}

void EventHandler::currencyBalanceChanged(string &itemId, int balance) {
    
    printf("balance: %d", cocos2dx_StoreInventory::getCurrencyBalance("currency_points"));
    
}

void EventHandler::goodBalanceChanged(string &itemId, int balance) {

}
