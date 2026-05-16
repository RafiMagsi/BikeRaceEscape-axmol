//
//  PZGGameplayHelicopterGame.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#ifndef __ProjectMayhem__PZGGameplayHelicopterGame__
#define __ProjectMayhem__PZGGameplayHelicopterGame__

#include <iostream>
#include "PZGGameInfoObject.h"

#define C_GAMEPLAY_HELICOPTER_CLASS_NAME "PZGameplayHelicopterGame"

class PZGGameplayHelicopterGame : public PZGGameInfoObject
{
public:
    
    PZGGameplayHelicopterGame();
    ~PZGGameplayHelicopterGame();


    float forceUp;
    float gravity;
    
    
    static PZGGameplayHelicopterGame* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameplayHelicopterGame__) */
