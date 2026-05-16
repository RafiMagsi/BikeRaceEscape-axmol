//
//  PZGGameplayJumpingGame.cpp
//  ProjectMayhem
//
//  Created by Amit Srivastava on 29/04/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameplayJumpingGame.h"
#include <string.h>
#include <stdio.h>

PZGGameplayJumpingGame::PZGGameplayJumpingGame(){
    
}

PZGGameplayJumpingGame::~PZGGameplayJumpingGame(){
    
}

PZGGameplayJumpingGame* PZGGameplayJumpingGame::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayJumpingGame *obj = new PZGGameplayJumpingGame();
    
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
    obj->tiltScale = dictionary->valueForKey("tiltScale")->floatValue();
    obj->useAccelerometer = dictionary->valueForKey("useAccelerometer")->boolValue();
    
    ax::__Dictionary* dict;
    ax::__Array* array;
    ax::Object *arrayObj;
    
    dict = ( ax::__Dictionary* )dictionary->valueForKey("platfrom1");
    if (dict) {
        obj->platform1Art = PZGArtObject::createWithDictionary( dict );
    }
    
    arrayObj = (ax::Object*)dictionary->valueForKey("platfrom1_cp");
    array = dynamic_cast<ax::__Array*>(arrayObj);
    if (array) {
        ax::__String *s;
        s = ( ax::__String*)array->objectAtIndex( 0 );
        obj->platform1Cp[ 0 ] = s->pointValue();
        s = ( ax::__String*)array->objectAtIndex( 1 );
        obj->platform1Cp[ 1 ] = s->pointValue();
    }
    
    dict = ( ax::__Dictionary* )dictionary->valueForKey("platfrom2");
    if (dict) {
        obj->platform2Art = PZGArtObject::createWithDictionary( dict );
    }
    
    arrayObj = (ax::Object*)dictionary->valueForKey("platfrom2_cp");
    array = dynamic_cast<ax::__Array*>(arrayObj);
    if (array) {
        ax::__String *s;
        s = ( ax::__String*)array->objectAtIndex( 0 );
        obj->platform2Cp[ 0 ] = s->pointValue();
        s = ( ax::__String*)array->objectAtIndex( 1 );
        obj->platform2Cp[ 1 ] = s->pointValue();
    }
    
    dict = ( ax::__Dictionary* )dictionary->valueForKey("platfrom3");
    if (dict) {
        obj->platform3Art = PZGArtObject::createWithDictionary( dict );
    }
    
    arrayObj = (ax::Object*)dictionary->valueForKey("platfrom3_cp");
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
