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
    ax::__Array* array = (ax::__Array*)sd->artResource->objectForKey( "Characters" );
    
    for (int i=0; i < array->count(); i++){
        PZGArtCharacter *character = (PZGArtCharacter*) array->objectAtIndex( i );
        if (strcmp(character->storeId->getCString(), productId.c_str()) == 0) {
            
            ax::Director* director = ax::Director::getInstance();
            ax::Scene* scene = director->getRunningScene();
            PZGCharacterMenu *menu = (PZGCharacterMenu*)scene->getChildren().at( 0 );
            
            menu->purchaseComplete( i );
            return;
        }
    }
    
    //check for RemoveAd IAP
    array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
    if ( strcmp(iapInfo->removeAd_id->getCString(), productId.c_str())==0 ) {
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director->getRunningScene();
        PZGBaseMenuScene *menu = (PZGBaseMenuScene*)scene->getChildren().at( 0 );
        menu->upgradeComplete();
        return;
    }
    
    //check for KidMode
    if ( strcmp( iapInfo->kidMode_id->getCString(), productId.c_str() )==0 ) {
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director->getRunningScene();
        PZGBaseMenuScene *menu = (PZGBaseMenuScene*)scene->getChildren().at( 0 );
        menu->kidModePurchased();
        return;
    }

    //check for CoinPacks
    PZSettingsController *sc = PZSettingsController::shared();
    if ( strcmp(iapInfo->coinShop1_id->getCString(), productId.c_str()) == 0 ) {
        sc->coins += iapInfo->coinShop1_value;
        sc->save();
    }
    else
    if (strcmp(iapInfo->coinShop2_id->getCString(), productId.c_str()) == 0){
        sc->coins += iapInfo->coinShop2_value;
        sc->save();
    }
    else
    if (strcmp(iapInfo->coinShop3_id->getCString(), productId.c_str()) == 0){
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
    ax::__Array* array = (ax::__Array*)sd->artResource->objectForKey( "Characters" );
    for (int i=0; i < array->count(); i++){
        PZGArtCharacter *character = (PZGArtCharacter*) array->objectAtIndex( i );
        if (strcmp(character->storeId->getCString(), productId.c_str()) == 0) {
            
            PZSettingsController *sc = PZSettingsController::shared();
            sc->unlockPlayer( i );
            return;
        }
    }
    
    //check for RemoveAd IAP
    array = (ax::__Array*)sd->gameInfoResource->objectForKey("IAPSettings");
    PZGGameInfoIAP *iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
    if ( strcmp(iapInfo->removeAd_id->getCString(), productId.c_str())==0 ) {
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director->getRunningScene();
        if (scene) {
            PZGBaseMenuScene *menu = (PZGBaseMenuScene*)scene->getChildren().at( 0 );
            menu->upgradeComplete();
            return;
        }
    }
    
    //check for KidMode
    if ( strcmp( iapInfo->kidMode_id->getCString(), productId.c_str() )==0 ) {
        ax::Director* director = ax::Director::getInstance();
        ax::Scene* scene = director->getRunningScene();
        if (scene) {
            PZGBaseMenuScene *menu = (PZGBaseMenuScene*)scene->getChildren().at( 0 );
            menu->kidModePurchased();
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

