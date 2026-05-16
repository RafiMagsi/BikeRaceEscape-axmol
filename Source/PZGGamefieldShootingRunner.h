//
//  PZGGamefieldShootingRunner.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#ifndef __ProjectMayhem__PZGGamefieldShootingRunner__
#define __ProjectMayhem__PZGGamefieldShootingRunner__

#include <iostream>
#include "PZGGamefieldBasicRunner.h"

class PZGGamefieldShootingRunner : public PZGGamefieldBasicRunner{
    
public:

    static ax::Scene* scene();
    LAYER_CREATE_FUNC( PZGGamefieldShootingRunner );
  
    ax::Touch*           fire_touch;
    ax::Touch*           move_touch;
    
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesEnded(ax::__Set *pTouches, ax::Event *pEvent);
};


#endif /* defined(__ProjectMayhem__PZGGamefieldShootingRunner__) */
