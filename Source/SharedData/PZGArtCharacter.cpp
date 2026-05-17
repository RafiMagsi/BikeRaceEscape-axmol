//
//  PZGArtCharacter.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 10/30/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGArtCharacter.h"

PZGArtCharacter::PZGArtCharacter(){
    
}

PZGArtCharacter::~PZGArtCharacter(){
    
}


PZGArtCharacter* PZGArtCharacter::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGArtCharacter *artObj = new PZGArtCharacter();

    if (!dictionary) {
        AXLOGE("PZGArtCharacter::createWithDictionary: dictionary is null");
        return artObj;
    }
    
    artObj->name = (ax::__String*)dictionary-> valueForKey(ART_OBJECT_NAME);
    
    artObj->index = ((ax::__String*)dictionary-> valueForKey("index"))->intValue();
    
    artObj->className = dictionary->valueForKey(OBJECT_CLASS_NAME);
    artObj->key = (ax::__String*)dictionary->valueForKey(ART_OBJECT_KEY);
    artObj->no_of_sprites = (dictionary->valueForKey(ART_NO_OF_SPRITES))->intValue();
    
    artObj->scale = ( (ax::__String*)dictionary->objectForKey( "scale" ))->floatValue();
    artObj->animationSpeed = ( (ax::__String*)dictionary->objectForKey( "animationSpeed" ))->floatValue();
    
    artObj->price = ( (ax::__String*)dictionary->objectForKey( C_ART_CHARACTER_PRICE ))->intValue();
    artObj->bouncing = ( (ax::__String*)dictionary->objectForKey( C_ART_CHARACTER_BOUNCING ))->boolValue();
    
    artObj->desctiprion = dictionary->valueForKey( C_ART_CHARACTER_DESCRIPTION );
    artObj->storeId = (ax::__String*)dictionary->valueForKey( "storeId" );
    if (artObj->storeId) {
    }
    artObj->storeDescription = (ax::__String*)dictionary->valueForKey( "storeDescription" );
    if (artObj->storeDescription) {
    }

    artObj->enableIAP = ((ax::__String*)dictionary->valueForKey( "enableIAP" ))->boolValue();
    
    // collision is an array; must use objectForKey (valueForKey returns __String only)
    if (auto* array = dynamic_cast<ax::__Array*>(dictionary->objectForKey("collision"))) {
        const int max = std::min<int>(array->count(), (int)(sizeof(artObj->collision) / sizeof(artObj->collision[0])));
        for (int i = 0; i < max; i++) {
            if (auto* s = dynamic_cast<ax::__String*>(array->objectAtIndex(i))) {
                artObj->collision[i] = s->pointValue();
            }
        }
    }
    
    // nested objects are dictionaries; must use objectForKey
    if (auto* dict = dynamic_cast<ax::__Dictionary*>(dictionary->objectForKey("deathArtObj"))) {
        artObj->deathArtObj = PZGArtObject::createWithDictionary(dict);
    }
    if (auto* dict = dynamic_cast<ax::__Dictionary*>(dictionary->objectForKey("fireArtObj"))) {
        artObj->fireArtObj = PZGArtObject::createWithDictionary(dict);
    }
    if (auto* dict = dynamic_cast<ax::__Dictionary*>(dictionary->objectForKey("bulletArtObj"))) {
        artObj->bulletArtObj = PZGArtObject::createWithDictionary(dict);
    }
    if (auto* dict = dynamic_cast<ax::__Dictionary*>(dictionary->objectForKey("iconArtObj"))) {
        artObj->iconArtObj = PZGArtObject::createWithDictionary(dict);
    }
    if (auto* dict = dynamic_cast<ax::__Dictionary*>(dictionary->objectForKey("jumpArtObj"))) {
        artObj->jumpArtObj = PZGArtObject::createWithDictionary(dict);
    }
    
    
    
    return artObj;
}
