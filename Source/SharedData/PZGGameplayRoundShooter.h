//
//  PZGGameplayRoundShooter.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/9/13.
//
//

#ifndef __ProjectMayhem__PZGGameplayRoundShooter__
#define __ProjectMayhem__PZGGameplayRoundShooter__

#include <iostream>
#include "PZGGameInfoObject.h"

#define C_GAMEPLAY_ROUNDSHOOTER_CLASS_NAME "PZGameplayRoundShooter"

class PZGGameplayRoundShooter : public PZGGameInfoObject
{
public:
    
    PZGGameplayRoundShooter();
    ~PZGGameplayRoundShooter();
    
    float shootSpeed;
    float bulletSpeed;
        
    static PZGGameplayRoundShooter* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameplayRoundShooter__) */
