//
//  PZGGameplayRacingGame.h
//  ProjectMayhem
//
//  Created by Amit Srivastava on 20/05/13.
//
//

#ifndef __ProjectMayhem__PZGGameplayRacingGame__
#define __ProjectMayhem__PZGGameplayRacingGame__

#include "PZGGameInfoObject.h"
#include "PZGArtObject.h"

#define C_GAMEPLAY_RACINGGAME_CLASS_NAME "PZGameplayRacingGame"

class PZGGameplayRacingGame : public PZGGameInfoObject
{
public:
    
    PZGGameplayRacingGame();
    ~PZGGameplayRacingGame();
    
    float tiltScale;
    bool useAccelerometer;
    float boostMultiplier;
    PZGArtObject* boostupArtObject;
    
    static PZGGameplayRacingGame* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameplayRacingGame__) */
