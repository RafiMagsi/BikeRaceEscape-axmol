 //
//  BackgroundView.h
//  PZPlayer
//
//  Created by Nik Rudenko on 6/13/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#ifndef PZPlayer_PZBackgroundScene_h
#define PZPlayer_PZBackgroundScene_h

#define kBACKGROUND_MAX_LAYERS 16

#include "PZLegacyCompat.h"
#include "PZLegacyInputLayer.h"
#include "SharedData/PZGSharedData.h"
#include "Parallax/CCParallaxScrollNode.h"

#include "PZGCharacter.h"

class PZGBackgroundScene : public PZLegacyLayer
{
  
    
    ax::__Array* backgroundLayers;
    CCParallaxScrollNode *parallax[ kBACKGROUND_MAX_LAYERS ];
    int parallax_n;
    
    PZGCharacter* character;
    
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;
    Touch*           fire_touch;
    Touch*           move_touch;
    
    
public:
    bool    enabled;
    float   speed;
    
    bool    isOrientationLandscape;
    
    
    void update(float dt);
    
	virtual bool init();  
    static ax::Scene* scene();
	
    
	LAYER_CREATE_FUNC(PZGBackgroundScene);
    
    void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    void ccTouchesMoved(ax::__Set *pTouch, ax::Event *pEvent);
    void ccTouchesEnded(ax::__Set *pTouch, ax::Event *pEvent);
};


#endif
