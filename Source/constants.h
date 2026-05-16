//
//  constants.h
//  PZPlayer
//
//  Created by Nik Rudenko on 6/21/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#ifndef PZPlayer_constants_h
#define PZPlayer_constants_h

#define kCollisionOff 0.7

#define kBaseMenuItemTag 100

#define kUIIDCharacterMenuLeftButton 2
#define kUIIDCharacterMenuRightButton 1

#define kUIIDMainMenuGameCenter 2
#define kUIIDMainMenuRestoreIAP 4
#define kUIIDMainMenuUpgradeIAP 5
#define kUIIDMainMenuCoinShop   6
#define kUIIDMainMenuKidModeIAP 7
#define kUIIDMainMenuKidModeOff 8

#define kUIIDGameFieldPauseButton 0

#define KUIIDPauseMenuBackButton 1
#define KUIIDPauseMenuRestartButton 2
#define KUIIDPauseMenuContinueButton 3

#define kSoundIDMainMenuMusic   9
#define kSoundIDGameFieldMusic  8
#define kSoundIDButtonPressed   2
#define kSoundIDCoinPickUp      3
#define kSoundIDShootingSound   4
#define kSoundIDGameOver        1
#define kSoundIDObstacleDeath1  5
#define kSoundIDObstacleDeath2  6
#define kSoundIDObstacleDeath3  7
#define kSoundIDGameStart       0

float kPhysicsWorldScale();
float kDeviceScale();
ax::Size kDeviceWinSize();

void PZLog( const char* msg);

#endif
