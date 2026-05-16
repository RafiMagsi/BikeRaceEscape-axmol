//
//  PZGArtObstacle.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/7/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGArtObstacle.h"

PZGArtObstacle::PZGArtObstacle(){
    
}

PZGArtObstacle::~PZGArtObstacle(){
    
}


PZGArtObstacle* PZGArtObstacle::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGArtObstacle *obj = new PZGArtObstacle();
    
    obj->className = dictionary->valueForKey("className");
    obj->key = (ax::__String*)dictionary->valueForKey("key");
    obj->no_of_sprites = dictionary->valueForKey("no_of_sprites")->intValue();
    obj->index = dictionary->valueForKey("index")->intValue();
    
    obj->comingRatio = dictionary-> valueForKey("comingRatio")->floatValue();
    obj->health = dictionary-> valueForKey("health")->floatValue();
    obj->baseSpeed = dictionary-> valueForKey("baseSpeed")->floatValue();
    
    obj->scale = dictionary-> valueForKey("scale")->floatValue();
    
    obj->destroyCharacter = dictionary-> valueForKey("destroyCharacter")->boolValue();
    obj->collideCharacter = dictionary-> valueForKey("collideCharacter")->boolValue();
    
    obj->randomizeScale = dictionary-> valueForKey("randomizeScale")->boolValue();
    obj->randomizeRotation = dictionary-> valueForKey("randomizeRotation")->boolValue();
    obj->animateRotation = dictionary-> valueForKey("animateRotation")->boolValue();
    obj->bouncingAnimation = dictionary-> valueForKey("bouncingAnimation")->boolValue();


    obj->trajectory = dictionary-> valueForKey("trajectory")->intValue();
    
    ax::__Array* array = ( ax::__Array* )dictionary->valueForKey("collision");
    if (array) {
        for (int i=0; i < array->count(); i++) {
            ax::__String* s = (ax::__String*)array->objectAtIndex( i );
            obj->collision[ i ] = s->pointValue();
        }
    }
    
    obj->offset = dictionary->valueForKey("offset")->pointValue();
    
    ax::__Dictionary* dict;
    dict = ( ax::__Dictionary* )dictionary->valueForKey("deathArtObj");
    if (dict) {
        obj->deathArtObj = PZGArtObject::createWithDictionary( dict );
    }
    
    return obj;
}
