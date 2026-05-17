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

    if (!dictionary) {
        AXLOGE("PZGArtObstacle::createWithDictionary: dictionary is null");
        return obj;
    }

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
    
    // collision is an array; must use objectForKey (valueForKey returns __String only)
    if (auto* array = dynamic_cast<ax::__Array*>(dictionary->objectForKey("collision"))) {
        const int max = std::min<int>(array->count(), (int)(sizeof(obj->collision) / sizeof(obj->collision[0])));
        for (int i = 0; i < max; i++) {
            if (auto* s = dynamic_cast<ax::__String*>(array->objectAtIndex(i))) {
                obj->collision[i] = s->pointValue();
            }
        }
    }
    
    obj->offset = dictionary->valueForKey("offset")->pointValue();
    
    // deathArtObj is a dictionary; must use objectForKey
    if (auto* dict = dynamic_cast<ax::__Dictionary*>(dictionary->objectForKey("deathArtObj"))) {
        obj->deathArtObj = PZGArtObject::createWithDictionary(dict);
    }
    
    return obj;
}
