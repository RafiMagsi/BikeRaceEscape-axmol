//
//  PZGameInfoGeneral.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/3/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameInfoGeneral.h"

PZGGameInfoGeneral::PZGGameInfoGeneral()
{
    
}
PZGGameInfoGeneral::~PZGGameInfoGeneral()
{
    
}

PZGGameInfoGeneral*  PZGGameInfoGeneral::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameInfoGeneral *obj = new PZGGameInfoGeneral();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    obj->isPortraitMode = dictionary->valueForKey("isPortraitMode")->boolValue();
    obj->isDebugMode = dictionary->valueForKey("isDebugMode")->boolValue();
    obj->isGameCenterEnabled = dictionary->valueForKey("gameCenter")->boolValue();
   
    obj->neocortexURL = (ax::__String*)dictionary->valueForKey("neocortexURL");
    obj->neocortexURL->retain();

    obj->neocortexURLAndroid = (ax::__String*)dictionary->valueForKey("neocortexURLAndroid");
    obj->neocortexURLAndroid->retain();

    obj->bundleid = (ax::__String*)dictionary->valueForKey("bundleid");
    obj->bundleid->retain();
    
    return obj;
    
}
