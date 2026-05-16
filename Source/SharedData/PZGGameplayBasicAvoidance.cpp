//
//  PZGGameplayBasicAvoidance.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#include "PZGGameplayBasicAvoidance.h"

PZGGameplayBasicAvoidance::PZGGameplayBasicAvoidance(){
    
}

PZGGameplayBasicAvoidance::~PZGGameplayBasicAvoidance(){
    
}

PZGGameplayBasicAvoidance* PZGGameplayBasicAvoidance::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayBasicAvoidance *obj = new PZGGameplayBasicAvoidance();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();

    obj->speedScaleX = dictionary->valueForKey("speedScaleX")->floatValue();
    obj->speedScaleY = dictionary->valueForKey("speedScaleY")->floatValue();
    
    
    return obj;
}