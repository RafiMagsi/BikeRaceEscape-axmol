//
//  PZGGameplayJumpingGame.h
//  ProjectMayhem
//
//  Created by Amit Srivastava on 29/04/13.
//
//

#ifndef __ProjectMayhem__PZGGameplayJumpingGame__
#define __ProjectMayhem__PZGGameplayJumpingGame__

#include "PZGGameInfoObject.h"
#include "PZGArtObject.h"

#define C_GAMEPLAY_JUMPING_CLASS_NAME "PZGameplayJumpingGame"

class PZGGameplayJumpingGame : public PZGGameInfoObject
{
public:
    
    PZGGameplayJumpingGame();
    ~PZGGameplayJumpingGame();
    
    bool randomLevelGeneration;
    
    float characterJumpHeight;
    float platfromsBetweenDistance;
    float platfromsBetweenIncreaser;
    
    float tiltScale;
    bool useAccelerometer;
    
    PZGArtObject*    platform1Art;
    ax::Vec2 platform1Cp[ 2 ];
    
    PZGArtObject*    platform2Art;
    ax::Vec2 platform2Cp[ 2 ];
    
    PZGArtObject*    platform3Art;
    ax::Vec2 platform3Cp[ 2 ];
    
    static PZGGameplayJumpingGame* createWithDictionary(ax::__Dictionary * dictionary);
    
};

#endif /* defined(__ProjectMayhem__PZGGameplayJumpingGame__) */
