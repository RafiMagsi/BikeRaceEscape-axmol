//
//  PZGameplayBasicRunner.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameplayBasicRunner.h"
#include <string.h>
#include <stdio.h>

PZGGameplayBasicRunner::PZGGameplayBasicRunner(){
    
}

PZGGameplayBasicRunner::~PZGGameplayBasicRunner(){
    
}

PZGGameplayBasicRunner* PZGGameplayBasicRunner::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayBasicRunner *obj = new PZGGameplayBasicRunner();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->randomLevelGeneration = dictionary->valueForKey("randomLevelGeneration")->boolValue();
    
    obj->characterJumpHeight = dictionary->valueForKey("characterJumpHeight")->floatValue();
    obj->platfromsBetweenDistance = dictionary->valueForKey("platfromsBetweenDistance")->floatValue();
    obj->platfromsBetweenIncreaser = dictionary->valueForKey("platfromsBetweenIncreaser")->floatValue();
    obj->platfromsHeightSpread = dictionary->valueForKey("platfromsHeightSpread")->floatValue();
    
    ax::__Dictionary* dict = nullptr;
    ax::__Array* array = nullptr;
    ax::Object *arrayObj = nullptr;
    
    // NOTE: in our legacy compat layer, `valueForKey` returns only string-like values.
    // For nested dict/array objects coming from plist, always use `objectForKey`.
    dict = ( ax::__Dictionary* )dictionary->objectForKey("platfrom1");
    if (dict) {
        obj->platform1Art = PZGArtObject::createWithDictionary( dict );
    }
    
    arrayObj = (ax::Object*)dictionary->objectForKey("platfrom1_cp");
    array = dynamic_cast<ax::__Array*>(arrayObj);
    if (array) {
        ax::__String *s;
        s = ( ax::__String*)array->objectAtIndex( 0 );
        obj->platform1Cp[ 0 ] = s->pointValue();
        s = ( ax::__String*)array->objectAtIndex( 1 );
        obj->platform1Cp[ 1 ] = s->pointValue();
    }
    
    dict = ( ax::__Dictionary* )dictionary->objectForKey("platfrom2");
    if (dict) {
        obj->platform2Art = PZGArtObject::createWithDictionary( dict );
    }
    
    arrayObj = (ax::Object*)dictionary->objectForKey("platfrom2_cp");
    array = dynamic_cast<ax::__Array*>(arrayObj);
    if (array) {
        ax::__String *s;
        s = ( ax::__String*)array->objectAtIndex( 0 );
        obj->platform2Cp[ 0 ] = s->pointValue();
        s = ( ax::__String*)array->objectAtIndex( 1 );
        obj->platform2Cp[ 1 ] = s->pointValue();
    }
    
    dict = ( ax::__Dictionary* )dictionary->objectForKey("platfrom3");
    if (dict) {
        obj->platform3Art = PZGArtObject::createWithDictionary( dict );
    }
    
    arrayObj = (ax::Object*)dictionary->objectForKey("platfrom3_cp");
    array = dynamic_cast<ax::__Array*>(arrayObj);
    if (array) {
        ax::__String *s;
        s = ( ax::__String*)array->objectAtIndex( 0 );
        obj->platform3Cp[ 0 ] = s->pointValue();
        s = ( ax::__String*)array->objectAtIndex( 1 );
        obj->platform3Cp[ 1 ] = s->pointValue();
    }
    
    return obj;
}
