//
//  PZGGameplaySlowRunner.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/10/13.
//
//

#ifndef __ProjectMayhem__PZGGameplaySlowRunner__
#define __ProjectMayhem__PZGGameplaySlowRunner__

#include "PZGGameInfoObject.h"
#include "PZGArtObject.h"
#include "PZGGameplayBasicRunner.h"

#define C_GAMEPLAY_SLOWMOVERUNNER_CLASS_NAME "PZGameplaySlowMoveRunner"

class PZGGameplaySlowMoveRunner : public PZGGameplayBasicRunner
{
public:
    
    PZGGameplaySlowMoveRunner();
    ~PZGGameplaySlowMoveRunner();
    
    
    
    static PZGGameplaySlowMoveRunner* createWithDictionary(ax::__Dictionary * dictionary);
    
};


#endif /* defined(__ProjectMayhem__PZGGameplaySlowRunner__) */
