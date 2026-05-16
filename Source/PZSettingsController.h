//
//  SettingsController.h
//  ProjectZero
//
//  Created by Nik Rudenko on 9/3/12.
//
//

#ifndef __ProjectZero__SettingsController__
#define __ProjectZero__SettingsController__

#include "PZLegacyCompat.h"

USING_NS_AX;

#define kMaxPlayers 32

class PZSettingsController {
    int unlockedPlayers[ kMaxPlayers ];
    
public:
    
    PZSettingsController();
    
    static PZSettingsController* shared( );
    
    __String* documentPath;
    
    int characterId;
    
    int coins;
    bool removeAds;
    bool kidMode;
    bool kidModePurchased;
    
    void save();
    void load();
    
    void setDocumentPath( const char* path);
    
    void unlockPlayer( int playerId );
    void unlockAllPlayers();
    int isPlayerUnlocked(int playerId );
};

#endif /* defined(__ProjectZero__SettingsController__) */
