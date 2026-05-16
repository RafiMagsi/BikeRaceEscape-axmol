//
//  PZGGamefieldHelicopterGame.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#ifndef __ProjectMayhem__PZGGamefieldHelicopterGame__
#define __ProjectMayhem__PZGGamefieldHelicopterGame__

#include <iostream>
#include "PZGGameFieldScene.h"



class PZGGamefieldHelicopterGame : public PZGGameFieldScene{
    
    
public:

    bool characterForceUp;
    float force;
    
    float forceUpValue;
    float gravityValue;
    
    virtual bool init();
    static ax::Scene* scene();
    LAYER_CREATE_FUNC( PZGGamefieldHelicopterGame );
    
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;
    
    
    virtual void update(float dt);
    virtual void reset();
    
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesEnded(ax::__Set *pTouches, ax::Event *pEvent);
};

#endif /* defined(__ProjectMayhem__PZGGamefieldHelicopterGame__) */
