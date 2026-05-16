//
//  PZGGamefieldBasicAvoidance.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#ifndef __ProjectMayhem__PZGGamefieldBasicAvoidance__
#define __ProjectMayhem__PZGGamefieldBasicAvoidance__

#include <iostream>
#include "PZGGameFieldScene.h"

#include "PZGGamefieldPlatforms.h"


class PZGGamefieldBasicAvoidance : public PZGGameFieldScene{

public:

    virtual bool init();
    static ax::Scene* scene();

    LAYER_CREATE_FUNC( PZGGamefieldBasicAvoidance );
  
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;
    

    float speedScaleX;
    float speedScaleY;
    
    virtual void update(float dt);

    virtual void reset();
    
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesMoved(ax::__Set *pTouch, ax::Event *pEvent);

    
};

#endif /* defined(__ProjectMayhem__PZGGamefieldBasicAvoidance__) */
