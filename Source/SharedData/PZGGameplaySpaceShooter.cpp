//
//  PZGGameplaySpaceShooter.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#include "PZGGameplaySpaceShooter.h"

PZGGameplaySpaceShooter::PZGGameplaySpaceShooter(){
    
}

PZGGameplaySpaceShooter::~PZGGameplaySpaceShooter(){
    
}

PZGGameplaySpaceShooter* PZGGameplaySpaceShooter::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplaySpaceShooter *obj = new PZGGameplaySpaceShooter();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();

    obj->speedScaleX = dictionary->valueForKey("speedScaleX")->floatValue();
    obj->speedScaleY = dictionary->valueForKey("speedScaleY")->floatValue();

    obj->shootSpeed = dictionary->valueForKey("shootSpeed")->floatValue();
    obj->bulletSpeed = dictionary->valueForKey("bulletSpeed")->floatValue();

    obj->autoshooting = dictionary->valueForKey("autoshooting")->boolValue();
    
    return obj;
}