//
//  GameOverScene.h
//  PZPlayer
//
//  Created by Nik Rudenko on 6/24/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#ifndef PZPlayer_GameOverScene_h
#define PZPlayer_GameOverScene_h

#include "PZLegacyCompat.h"

#include "PZGBaseMenuScene.h"

class PZGGameOverScene : public PZGBaseMenuScene
{
    
    float gf_distance;
    
public:
    virtual bool init();  
    
    virtual void setDistance( int distance );
    
    void* baseScene;
    
	static ax::Scene* scene();
	
    ax::Label* font;     

    virtual void load(const char* keyName);
    
    virtual void menuBackCallback(Object* pSender);
	virtual void menuRestartCallback(Object* pSender);
    
    virtual void keyBackClicked();
    
    virtual void kidModePurchased();
    virtual void upgradeComplete();
    
    PZGSoundData *gameOverSound;
    
	LAYER_CREATE_FUNC( PZGGameOverScene );
};


#endif
