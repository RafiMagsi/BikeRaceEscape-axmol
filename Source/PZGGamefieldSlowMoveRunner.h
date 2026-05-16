//
//  PZGGamefieldSlowMoveRunner.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/10/13.
//
//

#ifndef __ProjectMayhem__PZGGamefieldSlowMoveRunner__
#define __ProjectMayhem__PZGGamefieldSlowMoveRunner__

#include "PZGGameFieldScene.h"

#include "PZGGamefieldPlatforms.h"
#include "SharedData/PZGSharedData.h"


class PZGGamefieldSlowMoveRunner : public PZGGameFieldScene{
    
    float gf_speed_base;
    
    PZGGamefieldPlatforms* platform;
    
    float isMoved;
    
public:
    
    virtual bool init();
    static ax::Scene* scene();
    LAYER_CREATE_FUNC( PZGGamefieldSlowMoveRunner );
    
    virtual void update(float dt);
    
    virtual void reset();
    
    virtual void ccTouchesBegan(__Set *pTouch, Event *pEvent);
    virtual void ccTouchesEnded(__Set *pTouch, Event *pEvent);    

    
};

#endif /* defined(__ProjectMayhem__PZGGamefieldSlowMoveRunner__) */
