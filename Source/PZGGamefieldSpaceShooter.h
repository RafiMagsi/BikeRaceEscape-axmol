//
//  PZGGamefieldSpaceShooter.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#ifndef __ProjectMayhem__PZGGamefieldSpaceShooter__
#define __ProjectMayhem__PZGGamefieldSpaceShooter__

#include <iostream>

#include "PZGGameFieldScene.h"
#include "PZGGamefieldPlatforms.h"

class PZGGamefieldSpaceShooter : public PZGGameFieldScene{
    
public:
    virtual bool init();
    static ax::Scene* scene();
    
    LAYER_CREATE_FUNC( PZGGamefieldSpaceShooter );
    
    
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;

    float speedScaleX;
    float speedScaleY;
    
    float autoshooting;
    
    Touch*           fire_touch;
    Touch*           move_touch;
    
    virtual void update(float dt);

    virtual void reset();
    
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesMoved(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesEnded(ax::__Set *pTouch, ax::Event *pEvent);
};

#endif /* defined(__ProjectMayhem__PZGGamefieldSpaceShooter__) */
