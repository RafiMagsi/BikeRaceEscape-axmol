//
//  PZGArtBackgroundPlay.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 3/15/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGArtBackgroundPlay.h"

PZGArtBackgroundPlay::PZGArtBackgroundPlay(){
    
}

PZGArtBackgroundPlay::~PZGArtBackgroundPlay(){
    
}

PZGArtBackgroundPlay* PZGArtBackgroundPlay::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGArtBackgroundPlay *artObj = new PZGArtBackgroundPlay();
    
    artObj->index = ((ax::__String*)dictionary->valueForKey("index"))->intValue();
    
    artObj->name = (ax::__String*)dictionary-> valueForKey(ART_OBJECT_NAME);
    
    artObj->className = dictionary->valueForKey(OBJECT_CLASS_NAME);
    artObj->key = ( ax::__String* )dictionary->valueForKey(ART_OBJECT_KEY);
    
    artObj->playGroundLayer = ((ax::__String*)dictionary->valueForKey("playGroundLayer"))->boolValue();
    
    artObj->speed = ( (ax::__String*)dictionary->objectForKey( C_ART_BG_SPEED ))->floatValue();
    artObj->position = ( (ax::__String*)dictionary->objectForKey( C_ART_BG_POSITION ))->floatValue();
    artObj->scale = ( (ax::__String*)dictionary->objectForKey( C_ART_BG_SCALE ))->floatValue();
    artObj->opacity = ( (ax::__String*)dictionary->objectForKey( "opacity" ))->floatValue();
    
    artObj->boundDown = ((ax::__String*)dictionary->objectForKey( "boundDown" ))->floatValue();
    artObj->boundLeft = ((ax::__String*)dictionary->objectForKey( "boundLeft" ))->floatValue();
    artObj->boundRight = ((ax::__String*)dictionary->objectForKey( "boundRight" ))->floatValue();
    artObj->boundUp = ((ax::__String*)dictionary->objectForKey( "boundUp" ))->floatValue();
    
    return artObj;
}
