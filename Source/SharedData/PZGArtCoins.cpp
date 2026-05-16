//
//  PZArtCoins.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/7/12.
//
//

#include "PZGArtCoins.h"

PZGArtCoins::PZGArtCoins(){
    
}

PZGArtCoins::~PZGArtCoins(){
    
}


PZGArtCoins* PZGArtCoins::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGArtCoins *obj = new PZGArtCoins();
    
    obj->className = dictionary->valueForKey("className");
    obj->key = (ax::__String*)dictionary->valueForKey("key");
    obj->key->retain();
    obj->no_of_sprites = dictionary->valueForKey("no_of_sprites")->intValue();
    obj->index = dictionary->valueForKey("index")->intValue();
    
    obj->comingRatio = dictionary->valueForKey("comingRatio")->floatValue();
    obj->reward = dictionary->valueForKey("reward")->intValue();
    obj->offset = dictionary->valueForKey("offset")->pointValue();
    obj->scale = dictionary->valueForKey("scale")->floatValue();

    return obj;
}
