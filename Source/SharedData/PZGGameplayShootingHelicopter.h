//
//  PZGGameplayShootingHelicopter.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#ifndef __ProjectMayhem__PZGGameplayShootingHelicopter__
#define __ProjectMayhem__PZGGameplayShootingHelicopter__

#include <iostream>

#include "PZGGameplayHelicopterGame.h"

#define C_GAMEPLAY_SHOOTINGHELICOPTER_CLASS_NAME "PZGameplayShootingHelicopter"

class PZGGameplayShootingHelicopter : public PZGGameplayHelicopterGame
{
public:
    
    PZGGameplayShootingHelicopter();
    ~PZGGameplayShootingHelicopter();
    
    float shootSpeed;
    float bulletSpeed;
    
    
    
    static PZGGameplayShootingHelicopter* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameplayShootingHelicopter__) */
