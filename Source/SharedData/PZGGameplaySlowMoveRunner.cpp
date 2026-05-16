//
//  PZGGameplaySlowRunner.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/10/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameplaySlowMoveRunner.h"

PZGGameplaySlowMoveRunner::PZGGameplaySlowMoveRunner(){
    
}

PZGGameplaySlowMoveRunner::~PZGGameplaySlowMoveRunner(){
    
}

PZGGameplaySlowMoveRunner* PZGGameplaySlowMoveRunner::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplaySlowMoveRunner *obj = new PZGGameplaySlowMoveRunner();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->randomLevelGeneration = dictionary->valueForKey("randomLevelGeneration")->boolValue();
    
    obj->characterJumpHeight = dictionary->valueForKey("characterJumpHeight")->floatValue();
    
    ax::__Dictionary* dict;
    ax::__Array* array;
    ax::Object *arrayObj;
    
    dict = ( ax::__Dictionary* )dictionary->valueForKey("platform1");
    if (dict) {
        obj->platform1Art = PZGArtObject::createWithDictionary( dict );
    }
    
    arrayObj = (ax::Object*)dictionary->valueForKey("platform1_cp");
    array = dynamic_cast<ax::__Array*>(arrayObj);
    if (array) {
        ax::__String *s;
        s = ( ax::__String*)array->objectAtIndex( 0 );
        obj->platform1Cp[ 0 ] = s->pointValue();
        s = ( ax::__String*)array->objectAtIndex( 1 );
        obj->platform1Cp[ 1 ] = s->pointValue();
    }
    
    return obj;
}
