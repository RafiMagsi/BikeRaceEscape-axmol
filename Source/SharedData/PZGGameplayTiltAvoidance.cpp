//
//  PZGGameplayTiltAvoidance.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/3/13.
//
//

#include "PZGGameplayTiltAvoidance.h"

PZGGameplayTiltAvoidance::PZGGameplayTiltAvoidance(){
    
}

PZGGameplayTiltAvoidance::~PZGGameplayTiltAvoidance(){
    
}

PZGGameplayTiltAvoidance* PZGGameplayTiltAvoidance::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayTiltAvoidance *obj = new PZGGameplayTiltAvoidance();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->tiltScale = dictionary->valueForKey("tiltScale")->floatValue();

    
    return obj;
}