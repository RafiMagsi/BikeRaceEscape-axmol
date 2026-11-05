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

    // Jump input forgiveness: a tap that lands slightly before touchdown is remembered for a few frames.
    float _jumpBuffer = 0.0f;
    bool  _touchHeld = false;
    void applyJumpApex();     // tells the platform generator how high the rider can really jump on this device
    
public:
    
    virtual bool init();
    static ax::Scene* scene();
    LAYER_CREATE_FUNC( PZGGamefieldBasicRunner );

    virtual void update(float dt);

    virtual void reset();

    virtual PZGGamefieldPlatforms* platformLayer() { return platform; }
    
    virtual void ccTouchesBegan(__Set *pTouch, Event *pEvent);
    virtual void ccTouchesEnded(__Set *pTouch, Event *pEvent);
};

#endif /* defined(__ProjectMayhem__PZGamefieldBasicRunner__) */
