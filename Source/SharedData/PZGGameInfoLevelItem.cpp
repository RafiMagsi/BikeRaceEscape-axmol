//
//  PZGGameInfoLevelItem.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 4/23/13.
//
//

#include "PZGGameInfoLevelItem.h"

PZGGameInfoLevelItem::PZGGameInfoLevelItem()
{
    
}
PZGGameInfoLevelItem::~PZGGameInfoLevelItem()
{
    
}

PZGGameInfoLevelItem*  PZGGameInfoLevelItem::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameInfoLevelItem *obj = new PZGGameInfoLevelItem();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->position = dictionary->valueForKey("position")->pointValue();
    obj->rotation = dictionary->valueForKey("rotation")->floatValue();
    obj->scale = dictionary->valueForKey("scale")->pointValue();
    
    obj->infoObjIndex = dictionary->valueForKey("infoObjIndex")->intValue();
    obj->infoObjKey = (ax::__String*)dictionary->valueForKey("infoObjKey");
    if (obj->infoObjKey) {
        obj->infoObjKey->retain();
    }
    
    return obj;
    
}