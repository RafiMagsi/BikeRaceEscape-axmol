//
//  PauseMenuScene.h
//  PZPlayer
//
//  Created by Nik Rudenko on 6/29/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#ifndef PZPlayer_PauseMenuScene_h
#define PZPlayer_PauseMenuScene_h

#include "PZLegacyCompat.h"
#include "PZGBaseMenuScene.h"
#include "SharedData/PZGSoundData.h"

class PZGPauseMenuScene : public PZGBaseMenuScene
{

    PZGSoundData *buttonClickedSound;
    
public:
    virtual bool init();  
        
    void* baseScene;
    
	static ax::Scene* scene();
	        
    virtual void menuBackCallback(Ref* pSender);
	virtual void menuRestartCallback(Ref* pSender);
	virtual void menuContinueCallback(Ref* pSender);    
    
    virtual void load(const char* keyName);
    virtual void onEnterTransitionDidFinish();

    virtual void kidModePurchased();
    virtual void upgradeComplete();

    virtual void keyBackClicked();
    
	LAYER_CREATE_FUNC( PZGPauseMenuScene );
};

#endif
