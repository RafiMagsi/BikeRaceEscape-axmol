//
//  PZGGameplayMegaJumpGame.cpp
//  ProjectMayhem
//
//  Created by Amit Srivastava on 15/05/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameplayMegaJumpGame.h"

PZGGameplayMegaJumpGame::PZGGameplayMegaJumpGame(){
    
}

PZGGameplayMegaJumpGame::~PZGGameplayMegaJumpGame(){
    
}

PZGGameplayMegaJumpGame* PZGGameplayMegaJumpGame::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayMegaJumpGame *obj = new PZGGameplayMegaJumpGame();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->speedScaleX = dictionary->valueForKey("speedScaleX")->floatValue();
    obj->speedScaleY = dictionary->valueForKey("speedScaleY")->floatValue();
    
    obj->tiltScale = dictionary->valueForKey("tiltScale")->floatValue();
    obj->useAccelerometer = dictionary->valueForKey("useAccelerometer")->boolValue();
    
    return obj;
}
