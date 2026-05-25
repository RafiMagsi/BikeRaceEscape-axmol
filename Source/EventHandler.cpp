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
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director ? director->getRunningScene() : nullptr;
        PZGBaseMenuScene* menu = nullptr;
        if (scene) {
            const auto& children = scene->getChildren();
            for (auto* c : children) {
                if (auto* m = dynamic_cast<PZGBaseMenuScene*>(c)) {
                    menu = m;
                    break;
                }
            }
        }

        if (menu) {
            menu->upgradeComplete();
        } else {
            // Fallback: apply effect without UI layer (prevents crashes when running scene is gameplay).
            AXLOGW("[IAP] marketPurchase: PZGBaseMenuScene not found; applying RemoveAds via settings only");
            PZSettingsController* sc = PZSettingsController::shared();
            if (sc) {
                sc->removeAds = true;
                sc->save();
            }
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
            PZ::AdsController::shared()->setAdsEnabled(false);
#endif
        }
        return;
    }
    
    //check for KidMode
    const char* kidModeId = (iapInfo->kidMode_id ? iapInfo->kidMode_id->getCString() : "");
    if (kidModeId[0] != '\0' && strcmp(kidModeId, productId.c_str()) == 0) {
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director ? director->getRunningScene() : nullptr;
        PZGBaseMenuScene* menu = nullptr;
        if (scene) {
            const auto& children = scene->getChildren();
            for (auto* c : children) {
                if (auto* m = dynamic_cast<PZGBaseMenuScene*>(c)) {
                    menu = m;
                    break;
                }
            }
        }
        if (menu) {
            menu->kidModePurchased();
        } else {
            AXLOGW("[IAP] marketPurchase: PZGBaseMenuScene not found; applying KidMode via settings only");
            PZSettingsController* sc = PZSettingsController::shared();
            if (sc) {
                sc->kidMode = true;
                sc->kidModePurchased = true;
                sc->save();
            }
        }
        return;
    }

    //check for CoinPacks
    PZSettingsController *sc = PZSettingsController::shared();
    const char* coin1 = (iapInfo->coinShop1_id ? iapInfo->coinShop1_id->getCString() : "");
    const char* coin2 = (iapInfo->coinShop2_id ? iapInfo->coinShop2_id->getCString() : "");
    const char* coin3 = (iapInfo->coinShop3_id ? iapInfo->coinShop3_id->getCString() : "");
    if (coin1[0] != '\0' && strcmp(coin1, productId.c_str()) == 0) {
        sc->coins += iapInfo->coinShop1_value;
        sc->save();
    }
    else
    if (coin2[0] != '\0' && strcmp(coin2, productId.c_str()) == 0) {
        sc->coins += iapInfo->coinShop2_value;
        sc->save();
    }
    else
    if (coin3[0] != '\0' && strcmp(coin3, productId.c_str()) == 0) {
        sc->coins += iapInfo->coinShop3_value;
        sc->save();
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
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director->getRunningScene();
        if (scene) {
            PZGBaseMenuScene* menu = nullptr;
            const auto& children = scene->getChildren();
            for (auto* c : children) {
                if (auto* m = dynamic_cast<PZGBaseMenuScene*>(c)) { menu = m; break; }
            }
            if (menu) menu->upgradeComplete();
            return;
        }
    }
    
    //check for KidMode
    const char* kidModeId = (iapInfo->kidMode_id ? iapInfo->kidMode_id->getCString() : "");
    if ( kidModeId[0] != '\0' && strcmp( kidModeId, productId.c_str() )==0 ) {
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director->getRunningScene();
        if (scene) {
            PZGBaseMenuScene* menu = nullptr;
            const auto& children = scene->getChildren();
            for (auto* c : children) {
                if (auto* m = dynamic_cast<PZGBaseMenuScene*>(c)) { menu = m; break; }
            }
            if (menu) menu->kidModePurchased();
            return;
        }
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
