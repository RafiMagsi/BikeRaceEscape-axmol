//
//  PZGGameplayShootingHelicopter.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameplayShootingHelicopter.h"

PZGGameplayShootingHelicopter::PZGGameplayShootingHelicopter(){
    
}

PZGGameplayShootingHelicopter::~PZGGameplayShootingHelicopter(){
    
}

PZGGameplayShootingHelicopter* PZGGameplayShootingHelicopter::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayShootingHelicopter *obj = new PZGGameplayShootingHelicopter();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->forceUp = dictionary->valueForKey("forceUp")->floatValue();
    obj->gravity = dictionary->valueForKey("gravity")->floatValue();
    
    obj->shootSpeed = dictionary->valueForKey("shootSpeed")->floatValue();
    obj->bulletSpeed = dictionary->valueForKey("bulletSpeed")->floatValue();
    
    return obj;
}
