//
//  PZGArtBackground.cpp
//  ProjectZero
//
//  Created by Nik Rudenko on 10/10/12.
//
//

#include "PZGArtBackground.h"

PZGArtBackground::PZGArtBackground(){
    
}

PZGArtBackground::~PZGArtBackground(){
    
}

PZGArtBackground* PZGArtBackground::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGArtBackground *artObj = new PZGArtBackground();
    
    artObj->index = ((ax::__String*)dictionary->valueForKey("index"))->intValue();
    
    artObj->name = (ax::__String*)dictionary-> valueForKey(ART_OBJECT_NAME);
    artObj->name->retain();
    
    artObj->className = dictionary->valueForKey(OBJECT_CLASS_NAME);
    artObj->key = ( ax::__String* )dictionary->valueForKey(ART_OBJECT_KEY);
    artObj->key->retain();
    
    artObj->playGroundLayer = ((ax::__String*)dictionary->valueForKey("playGroundLayer"))->boolValue();
    
    artObj->speed = ( (ax::__String*)dictionary->objectForKey( C_ART_BG_SPEED ))->floatValue();
    artObj->position = ( (ax::__String*)dictionary->objectForKey( C_ART_BG_POSITION ))->floatValue();
    artObj->scale = ( (ax::__String*)dictionary->objectForKey( C_ART_BG_SCALE ))->floatValue();
    artObj->opacity = ( (ax::__String*)dictionary->objectForKey( "opacity" ))->floatValue();
    
    return artObj;
}