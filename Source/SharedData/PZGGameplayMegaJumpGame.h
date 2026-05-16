//
//  PZGGameplayMegaJumpGame.h
//  ProjectMayhem
//
//  Created by Amit Srivastava on 15/05/13.
//
//

#ifndef __ProjectMayhem__PZGGameplayMegaJumpGame__
#define __ProjectMayhem__PZGGameplayMegaJumpGame__

#include <iostream>
#include "PZGGameInfoObject.h"

#define C_GAMEPLAY_MEGAJUMP_CLASS_NAME "PZGameplayMegaJumpGame"

class PZGGameplayMegaJumpGame : public PZGGameInfoObject
{
public:
    
    PZGGameplayMegaJumpGame();
    ~PZGGameplayMegaJumpGame();
    
    float speedScaleX;
    float speedScaleY;
    
    float tiltScale;
    bool useAccelerometer;
    
    static PZGGameplayMegaJumpGame* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameplayMegaJumpGame__) */
