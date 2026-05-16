//
//  PZSettingsController.cpp
//  ProjectZero
//
//  Created by Nik Rudenko on 9/3/12.
//
//

#include "PZSettingsController.h"
#include "constants.h"
#include "SharedData/PZGSharedData.h"

static PZSettingsController* s_pSettingController = NULL;

PZSettingsController* PZSettingsController::shared(  ){
    if ( s_pSettingController == NULL) {
        s_pSettingController = new PZSettingsController();
        s_pSettingController->coins = 0;
        s_pSettingController->characterId = 0;
        s_pSettingController->kidMode = false;
        
        for (int i=0; i < kMaxPlayers; i++) {
            s_pSettingController->unlockedPlayers[ i ] = 0;
        }
        s_pSettingController->load();
        
        
    }
    return s_pSettingController;
}

PZSettingsController::PZSettingsController(){

}

void PZSettingsController::setDocumentPath( const char* path){
    documentPath = __String::create( path );
    documentPath->retain();
}

void PZSettingsController::save(){
    
#if ( CC_TARGET_PLATFORM == CC_PLATFORM_MAC )
    const char *bundleid = "com.myname.bundleid";
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    __Array *array = (__Array*)sd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    if (array) {
        PZGGameInfoGeneral *gameInfoGeneral = (PZGGameInfoGeneral*)array->objectAtIndex(0);
        if (gameInfoGeneral->bundleid) {
            bundleid = gameInfoGeneral->bundleid->getCString();
        }
    }
    
    
    char buffer[256];
    
    CCUserDefault* ud = CCUserDefault::sharedUserDefault();
    sprintf(buffer, "%s__%s", bundleid, "PMP_Coins");
    ud->setFloatForKey(buffer, coins);
    sprintf(buffer, "%s__%s", bundleid, "PMP_RemoveAds");
    ud->setBoolForKey(buffer, removeAds);
    sprintf(buffer, "%s__%s", bundleid, "PMP_KidMode");
    ud->setBoolForKey(buffer, kidMode);
    sprintf(buffer, "%s__%s", bundleid, "PMP_KidModePurchased");
    ud->setBoolForKey(buffer, kidModePurchased);
    
    for (int i=0; i < kMaxPlayers; i++) {
        char c[256];
        sprintf(c, "%s__PMP_Character_%d", bundleid, i);
        ud->setIntegerForKey( c , unlockedPlayers[ i ]);
    }
    ud->flush();
    
#else
    
    CCUserDefault* ud = CCUserDefault::sharedUserDefault();
    ud->setFloatForKey("PMP_Coins", coins);
    ud->setBoolForKey("PMP_RemoveAds", removeAds);
    ud->setBoolForKey("PMP_KidMode", kidMode);
    ud->setBoolForKey("PMP_KidModePurchased", kidModePurchased);
    
    for (int i=0; i < kMaxPlayers; i++) {
        char c[64];
        sprintf(c, "PMP_Character_%d", i);
        ud->setIntegerForKey( c , unlockedPlayers[ i ]);
    }
    ud->flush();
    
#endif
}

void PZSettingsController::load(){
    
#if ( CC_TARGET_PLATFORM == CC_PLATFORM_MAC ) 
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    __Array *array = (__Array*)sd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral *gameInfoGeneral = (PZGGameInfoGeneral*)array->objectAtIndex(0);
    
    const char *bundleid = "com.myname.bundleid";
    if (gameInfoGeneral->bundleid) {
        bundleid = gameInfoGeneral->bundleid->getCString();
    }
    
    
    char buffer[256];
    CCUserDefault* ud = CCUserDefault::sharedUserDefault();

    sprintf(buffer, "%s__%s", bundleid, "PMP_Coins");
    coins = ud->getFloatForKey(buffer);
    sprintf(buffer, "%s__%s", bundleid, "PMP_RemoveAds");
    removeAds =ud->getBoolForKey(buffer);
    sprintf(buffer, "%s__%s", bundleid, "PMP_KidMode");
    kidMode = ud->getBoolForKey(buffer);
    sprintf(buffer, "%s__%s", bundleid, "PMP_KidModePurchased");
    kidModePurchased = ud->getBoolForKey(buffer);
    
    for (int i=0; i < kMaxPlayers; i++) {
        char c[256];
        sprintf(c, "%s__PMP_Character_%d", bundleid, i);
        unlockedPlayers[ i ] = ud->getIntegerForKey( c );
    }
    unlockedPlayers[ 0 ] = 1;
#else
    
    CCUserDefault* ud = CCUserDefault::sharedUserDefault();
    
    coins = ud->getFloatForKey("PMP_Coins");
    removeAds =ud->getBoolForKey("PMP_RemoveAds");
    kidMode = ud->getBoolForKey("PMP_KidMode");
    kidModePurchased = ud->getBoolForKey("PMP_KidModePurchased");
    
    for (int i=0; i < kMaxPlayers; i++) {
        char c[64];
        sprintf(c, "PMP_Character_%d", i);
        unlockedPlayers[ i ] = ud->getIntegerForKey( c );
    }
    unlockedPlayers[ 0 ] = 1;
    
#endif
}

void PZSettingsController::unlockAllPlayers(){
    for (int i=0; i < kMaxPlayers; i++){
        unlockPlayer( i );
    }
}

void PZSettingsController::unlockPlayer( int playerId ){
    unlockedPlayers[ playerId ]  = 1;
    save();
}
int PZSettingsController::isPlayerUnlocked(int playerId ){
    return unlockedPlayers[ playerId ];
}
