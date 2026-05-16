//
//  PZGGamefieldMegaJumpGame.h
//  ProjectMayhem
//
//  Created by Amit Srivasatava on 15/05/13.
//
//

#ifndef __ProjectMayhem__PZGGamefieldMegaJumpGame__
#define __ProjectMayhem__PZGGamefieldMegaJumpGame__

#include <iostream>
#include "PZGGameFieldScene.h"

#include "PZGGamefieldPlatforms.h"


class PZGGamefieldMegaJumpGame : public PZGGameFieldScene{
    
public:
    
    virtual bool init();
    static ax::Scene* scene();
    
    LAYER_CREATE_FUNC( PZGGamefieldMegaJumpGame );
    
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;
    
    
    float speedScaleX;
    float speedScaleY;
    
    virtual void update(float dt);
    
    virtual void reset();
    
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void didAccelerate(ax::Acceleration* pAccelerationValue);
    
    float tiltScale;
    bool useAccelerometer;
    
};

#endif /* defined(__ProjectMayhem__PZGGamefieldMegaJumpGame__) */
