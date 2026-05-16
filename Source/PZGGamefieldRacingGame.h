//
//  PZGGamefieldRacingGame.h
//  ProjectMayhem
//
//  Created by Amit Srivastava on 20/05/13.
//
//

#ifndef __ProjectMayhem__PZGGamefieldRacingGame__
#define __ProjectMayhem__PZGGamefieldRacingGame__

#include <iostream>
#include "PZGGameFieldScene.h"

class PZGGamefieldRacingGame : public PZGGameFieldScene{
    
    ax::Sprite*  s_body_boostUp;
    
    bool isTouched;
    
public:
    
    virtual bool init();
    static ax::Scene* scene();
    
    LAYER_CREATE_FUNC( PZGGamefieldRacingGame );
    
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;
    
    float tiltScale;
    bool useAccelerometer;
    float boostMultiplier;
    
    virtual void update(float dt);
    
    virtual void didAccelerate(ax::Acceleration* pAccelerationValue);
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesEnded(ax::__Set *pTouch, ax::Event *pEvent);
    
    
};

#endif /* defined(__ProjectMayhem__PZGGamefieldRacingGame__) */
