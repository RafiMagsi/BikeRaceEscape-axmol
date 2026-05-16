//
//  PZGGameplayRoundShooter.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/9/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameplayRoundShooter.h"

PZGGameplayRoundShooter::PZGGameplayRoundShooter(){
    
}

PZGGameplayRoundShooter::~PZGGameplayRoundShooter(){
    
}

PZGGameplayRoundShooter* PZGGameplayRoundShooter::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayRoundShooter *obj = new PZGGameplayRoundShooter();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->shootSpeed = dictionary->valueForKey("shootSpeed")->floatValue();
    obj->bulletSpeed = dictionary->valueForKey("bulletSpeed")->floatValue();
    
    
    return obj;
}
