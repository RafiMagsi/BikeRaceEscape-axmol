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
    
    artObj->name = (ax::__String*)dictionary-> valueForKey(ART_OBJECT_NAME);
    artObj->name->retain();
    
    artObj->index = ((ax::__String*)dictionary-> valueForKey("index"))->intValue();
    
    artObj->className = dictionary->valueForKey(OBJECT_CLASS_NAME);
    artObj->key = (ax::__String*)dictionary->valueForKey(ART_OBJECT_KEY);
    artObj->key->retain();
    artObj->no_of_sprites = (dictionary->valueForKey(ART_NO_OF_SPRITES))->intValue();
    
    artObj->scale = ( (ax::__String*)dictionary->objectForKey( "scale" ))->floatValue();
    artObj->animationSpeed = ( (ax::__String*)dictionary->objectForKey( "animationSpeed" ))->floatValue();
    
    artObj->price = ( (ax::__String*)dictionary->objectForKey( C_ART_CHARACTER_PRICE ))->intValue();
    artObj->bouncing = ( (ax::__String*)dictionary->objectForKey( C_ART_CHARACTER_BOUNCING ))->boolValue();
    
    artObj->desctiprion = dictionary->valueForKey( C_ART_CHARACTER_DESCRIPTION );
    artObj->storeId = (ax::__String*)dictionary->valueForKey( "storeId" );
    if (artObj->storeId) {
        artObj->storeId->retain();
    }
    artObj->storeDescription = (ax::__String*)dictionary->valueForKey( "storeDescription" );
    if (artObj->storeDescription) {
        artObj->storeDescription->retain();
    }

    artObj->enableIAP = ((ax::__String*)dictionary->valueForKey( "enableIAP" ))->boolValue();
    
    ax::__Array* array = ( ax::__Array* )dictionary->valueForKey("collision");
    if (array) {
        for (int i=0; i < array->count(); i++) {
            ax::__String* s = (ax::__String*)array->objectAtIndex( i );
            artObj->collision[ i ] = s->pointValue();
        }
    }
    
    ax::__Dictionary* dict;
    
    dict = ( ax::__Dictionary* )dictionary->valueForKey("deathArtObj");
    if (dict) {
        artObj->deathArtObj = PZGArtObject::createWithDictionary( dict );
    }

    dict = ( ax::__Dictionary* )dictionary->valueForKey("fireArtObj");
    if (dict) {
        artObj->fireArtObj = PZGArtObject::createWithDictionary( dict );
    }

    dict = ( ax::__Dictionary* )dictionary->valueForKey("bulletArtObj");
    if (dict) {
        artObj->bulletArtObj = PZGArtObject::createWithDictionary( dict );
    }

    dict = ( ax::__Dictionary* )dictionary->valueForKey("iconArtObj");
    if (dict) {
        artObj->iconArtObj = PZGArtObject::createWithDictionary( dict );
    }
    
    dict = ( ax::__Dictionary* )dictionary->valueForKey("jumpArtObj");
    if (dict) {
        artObj->jumpArtObj = PZGArtObject::createWithDictionary( dict );
    }
    
    
    
    return artObj;
}
