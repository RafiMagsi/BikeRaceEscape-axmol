//
//  PZGGameplayShootingRunner.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#ifndef __ProjectMayhem__PZGGameplayShootingRunner__
#define __ProjectMayhem__PZGGameplayShootingRunner__

#include <iostream>

#include "PZGGameInfoObject.h"
#include "PZGGameplayBasicRunner.h"

#define C_GAMEPLAY_SHOOTINGRUNNER_CLASS_NAME "PZGameplayShootingRunner"

class PZGGameplayShootingRunner : public PZGGameplayBasicRunner
{
public:
    
    PZGGameplayShootingRunner();
    ~PZGGameplayShootingRunner();
    
    float shootSpeed;
    float bulletSpeed;
    
    static PZGGameplayShootingRunner* createWithDictionary(ax::__Dictionary * dictionary);
    
};
#endif /* defined(__ProjectMayhem__PZGGameplayShootingRunner__) */
