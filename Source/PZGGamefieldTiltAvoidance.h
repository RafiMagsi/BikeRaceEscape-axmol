//
//  PZGGamefieldTiltAvoidance.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/3/13.
//
//

#ifndef __ProjectMayhem__PZGGamefieldTiltAvoidance__
#define __ProjectMayhem__PZGGamefieldTiltAvoidance__

#include <iostream>
#include "PZGGameFieldScene.h"

class PZGGamefieldTiltAvoidance : public PZGGameFieldScene{
    
public:
    
    virtual bool init();
    static ax::Scene* scene();
    
    LAYER_CREATE_FUNC( PZGGamefieldTiltAvoidance );
    
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;
    
    float tiltScale;
    
    virtual void update(float dt);

    virtual void didAccelerate(ax::Acceleration* pAccelerationValue);
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesEnded(ax::__Set *pTouch, ax::Event *pEvent);
    
    
};

#endif /* defined(__ProjectMayhem__PZGGamefieldTiltAvoidance__) */
