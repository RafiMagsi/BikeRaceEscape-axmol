//
//  PZGameplayBasicRunner.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#ifndef __ProjectMayhem__PZGameplayBasicRunner__
#define __ProjectMayhem__PZGameplayBasicRunner__


#include "PZGGameInfoObject.h"
#include "PZGArtObject.h"

#define C_GAMEPLAY_BASICRUNNER_CLASS_NAME "PZGameplayBasicRunner"

class PZGGameplayBasicRunner : public PZGGameInfoObject
{
public:
    
    PZGGameplayBasicRunner();
    ~PZGGameplayBasicRunner();


    bool randomLevelGeneration;
    
    float characterJumpHeight;
    float platfromsBetweenDistance;
    float platfromsBetweenIncreaser;
    float platfromsHeightSpread;
    
    PZGArtObject*    platform1Art;
    ax::Vec2 platform1Cp[ 2 ];
    
    PZGArtObject*    platform2Art;
    ax::Vec2 platform2Cp[ 2 ];
    
    PZGArtObject*    platform3Art;
    ax::Vec2 platform3Cp[ 2 ];
    
    static PZGGameplayBasicRunner* createWithDictionary(ax::__Dictionary * dictionary);
    
};


#endif /* defined(__ProjectMayhem__PZGameplayBasicRunner__) */
