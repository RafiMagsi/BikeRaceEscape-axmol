//
//  PZGGameplayHelicopterGame.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#include "PZGGameplayHelicopterGame.h"

PZGGameplayHelicopterGame::PZGGameplayHelicopterGame(){
    
}

PZGGameplayHelicopterGame::~PZGGameplayHelicopterGame(){
    
}

PZGGameplayHelicopterGame* PZGGameplayHelicopterGame::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayHelicopterGame *obj = new PZGGameplayHelicopterGame();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->forceUp = dictionary->valueForKey("forceUp")->floatValue();
    obj->gravity = dictionary->valueForKey("gravity")->floatValue();
    
    return obj;
}