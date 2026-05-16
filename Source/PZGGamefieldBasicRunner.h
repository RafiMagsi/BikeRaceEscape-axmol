//
//  PZGamefieldBasicRunner.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#ifndef __ProjectMayhem__PZGamefieldBasicRunner__
#define __ProjectMayhem__PZGamefieldBasicRunner__

#include "PZGGameFieldScene.h"

#include "PZGGamefieldPlatforms.h"

#include "SharedData/PZGSharedData.h"



class PZGGamefieldBasicRunner : public PZGGameFieldScene{

    PZGGamefieldPlatforms* platform;
    
public:
    
    virtual bool init();
    static ax::Scene* scene();
    LAYER_CREATE_FUNC( PZGGamefieldBasicRunner );

    virtual void update(float dt);

    virtual void reset();
    
    virtual void ccTouchesBegan(__Set *pTouch, Event *pEvent);
    virtual void ccTouchesEnded(__Set *pTouch, Event *pEvent);
};

#endif /* defined(__ProjectMayhem__PZGamefieldBasicRunner__) */
