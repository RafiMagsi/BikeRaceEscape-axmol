//
//  PZGGameplayBasicAvoidance.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#ifndef __ProjectMayhem__PZGGameplayBasicAvoidance__
#define __ProjectMayhem__PZGGameplayBasicAvoidance__

#include <iostream>
#include "PZGGameInfoObject.h"

#define C_GAMEPLAY_BASICAVOIDANCE_CLASS_NAME "PZGameplayBasicAvoidance"

class PZGGameplayBasicAvoidance : public PZGGameInfoObject
{
public:
    
    PZGGameplayBasicAvoidance();
    ~PZGGameplayBasicAvoidance();

    float speedScaleX;
    float speedScaleY;
    
    static PZGGameplayBasicAvoidance* createWithDictionary(ax::__Dictionary * dictionary);
};


#endif /* defined(__ProjectMayhem__PZGGameplayBasicAvoidance__) */
