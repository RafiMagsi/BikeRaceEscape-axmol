//
//  PZGGamefieldShootingHelicopter.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#ifndef __ProjectMayhem__PZGGamefieldShootingHelicopter__
#define __ProjectMayhem__PZGGamefieldShootingHelicopter__

#include <iostream>
#include "PZGGamefieldHelicopterGame.h"


class PZGGamefieldShootingHelicopter : public PZGGamefieldHelicopterGame{
public:
    
    Touch*           fire_touch;
    Touch*           move_touch;
    
    static ax::Scene* scene();
    
    LAYER_CREATE_FUNC( PZGGamefieldShootingHelicopter );
    
    virtual bool init();
    
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesEnded(ax::__Set *pTouches, ax::Event *pEvent);
};


#endif /* defined(__ProjectMayhem__PZGGamefieldShootingHelicopter__) */
