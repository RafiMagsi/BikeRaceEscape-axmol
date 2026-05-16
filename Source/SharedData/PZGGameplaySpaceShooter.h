//
//  PZGGameplaySpaceShooter.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#ifndef __ProjectMayhem__PZGGameplaySpaceShooter__
#define __ProjectMayhem__PZGGameplaySpaceShooter__

#include <iostream>
#include "PZGGameInfoObject.h"

#define C_GAMEPLAY_SPACESHOOTER_CLASS_NAME "PZGameplaySpaceShooter"

class PZGGameplaySpaceShooter : public PZGGameInfoObject
{
public:
    
    PZGGameplaySpaceShooter();
    ~PZGGameplaySpaceShooter();
    
    float shootSpeed;
    float bulletSpeed;
    
    float speedScaleX;
    float speedScaleY;
    
    bool autoshooting;
    
    static PZGGameplaySpaceShooter* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameplaySpaceShooter__) */
