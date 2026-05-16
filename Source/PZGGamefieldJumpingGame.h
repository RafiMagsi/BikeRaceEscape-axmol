//
//  PZGGamefieldJumpingGame.h
//  ProjectMayhem
//
//  Created by Amit Srivastava on 29/04/13.
//
//

#ifndef __ProjectMayhem__PZGGamefieldJumpingGame__
#define __ProjectMayhem__PZGGamefieldJumpingGame__

#include "PZGGameFieldScene.h"

#include "PZGGamefieldJumpingPlatforms.h"

#include "SharedData/PZGSharedData.h"

class PZGGamefieldJumpingGame : public PZGGameFieldScene{
    
    PZGGamefieldJumpingPlatforms* platform;
    
    ax::Vec2   player_target_offset;
    
    bool isInit;
    float scrollDiff;

public:
    
    virtual bool init();
    static ax::Scene* scene();
    LAYER_CREATE_FUNC( PZGGamefieldJumpingGame );
    
    virtual void update(float dt);
    
    virtual void reset();
    
    virtual void ccTouchesBegan(__Set *pTouch, Event *pEvent);
    virtual void ccTouchesEnded(__Set *pTouch, Event *pEvent);
    virtual void didAccelerate(ax::Acceleration* pAccelerationValue);
    
    float tiltScale;
    bool useAccelerometer;
};

#endif /* defined(__ProjectMayhem__PZGGamefieldJumpingGame__) */

