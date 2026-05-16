//
//  CharacterMenu.h
//  ProjectZero
//
//  Created by Nik Rudenko on 8/24/12.
//
//

#ifndef __CHARCTERMENU_SCENE_H__
#define __CHARCTERMENU_SCENE_H__

#include "PZLegacyCompat.h"
#include "PZGCharacter.h"
#include "PZGBaseMenuScene.h"

#include "PZGGamefieldBasicRunner.h"
#include "PZGGamefieldBasicAvoidance.h"
#include "PZGGamefieldHelicopterGame.h"
#include "PZGGamefieldSpaceShooter.h"
#include "PZGGamefieldRoundShooter.h"
#include "PZGGamefieldTiltAvoidance.h"
#include "PZGGamefieldSlowMoveRunner.h"

#include "PZGGamefieldShootingRunner.h"
#include "PZGGamefieldShootingHelicopter.h"
#include "PZGGamefieldJumpingGame.h"
#include "PZGGamefieldMegaJumpGame.h"
#include "PZGGamefieldRacingGame.h"

USING_NS_AX;

class PZGCharacterMenu : public PZGBaseMenuScene
{
private:
    int         player_c;
    int         player_n;
    
    PZGCharacter*     player;
    Sprite*         player_sprite;
    
    Label *player_name;
    Label *player_price;
    Label *coins_label;

    
    PZGArtInterface *playerPosition;
    
    float counter;
    
public:
    void loadCharacter(int characterId);
    
	virtual bool init();
    
    static ax::Scene* scene();
    
    void purchaseComplete(int characterId );
    
    void update(float dd);
    
    virtual void load(const char* keyName);
    
  	virtual void startGameCallback(Object* pSender);
  	virtual void backCallback(Object* pSender);
  	virtual void leftCallback(Object* pSender);
  	virtual void rightCallback(Object* pSender);
    virtual void unlockCallback(Object* pSender);
    
    virtual void onEnterTransitionDidFinish();
    
    LAYER_CREATE_FUNC(PZGCharacterMenu);
    
    virtual void kidModePurchased();
    virtual void upgradeComplete();
    
    virtual void keyBackClicked();
    
};

#endif //__CHARCTERMENU_SCENE_H__
