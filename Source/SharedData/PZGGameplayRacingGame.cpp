//
//  PZGGameplayRacingGame.cpp
//  ProjectMayhem
//
//  Created by Amit Srivastava on 20/05/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameplayRacingGame.h"

PZGGameplayRacingGame::PZGGameplayRacingGame(){
    
}

PZGGameplayRacingGame::~PZGGameplayRacingGame(){
    
}

PZGGameplayRacingGame* PZGGameplayRacingGame::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameplayRacingGame *obj = new PZGGameplayRacingGame();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->gameSpeedMin = dictionary->valueForKey("gameSpeedMin")->floatValue();
    obj->gameSpeedMax = dictionary->valueForKey("gameSpeedMax")->floatValue();
    obj->gameSpeedIncreaser = dictionary->valueForKey("gameSpeedIncreaser")->floatValue();
    obj->gameTypeId = dictionary->valueForKey("typeId")->intValue();
    
    obj->tiltScale = dictionary->valueForKey("tiltScale")->floatValue();
    obj->useAccelerometer = dictionary->valueForKey("useAccelerometer")->boolValue();
    obj->boostMultiplier = dictionary->valueForKey("boostMultiplier")->floatValue();
    
    ax::__Dictionary* dict;
    
    dict = ( ax::__Dictionary* )dictionary->valueForKey("boostupArtObject");
    if (dict) {
        obj->boostupArtObject = PZGArtObject::createWithDictionary( dict );
       // obj->boostupArtObject->retain();
    }
    
    return obj;
}
