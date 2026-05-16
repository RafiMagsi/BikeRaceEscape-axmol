//
//  PZGGameplayTiltAvoidance.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/3/13.
//
//

#ifndef __ProjectMayhem__PZGGameplayTiltAvoidance__
#define __ProjectMayhem__PZGGameplayTiltAvoidance__

#include <iostream>
#include "PZGGameInfoObject.h"

#define C_GAMEPLAY_TILTAVOIDANCE_CLASS_NAME "PZGameplayTiltAvoidance"

class PZGGameplayTiltAvoidance : public PZGGameInfoObject
{
public:
    
    PZGGameplayTiltAvoidance();
    ~PZGGameplayTiltAvoidance();
    
    float tiltScale;
    
    static PZGGameplayTiltAvoidance* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameplayTiltAvoidance__) */
