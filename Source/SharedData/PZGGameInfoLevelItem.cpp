//
//  PZGGameInfoLevelItem.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 4/23/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameInfoLevelItem.h"

PZGGameInfoLevelItem::PZGGameInfoLevelItem()
{
    
}
PZGGameInfoLevelItem::~PZGGameInfoLevelItem()
{
    
}

PZGGameInfoLevelItem*  PZGGameInfoLevelItem::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameInfoLevelItem *obj = new PZGGameInfoLevelItem();
    
    if (!dictionary) {
        AXLOGE("PZGGameInfoLevelItem::createWithDictionary got null dictionary");
        return obj;
    }

    obj->name = dictionary->valueForKey("name");
    if (!obj->name) obj->name = ax::__String::create("");

    obj->className = dictionary->valueForKey("className");
    if (!obj->className) obj->className = ax::__String::create("");

    obj->key = dictionary->valueForKey("key");
    if (!obj->key) obj->key = ax::__String::create("");

    if (auto* vPos = dictionary->valueForKey("position")) {
        obj->position = vPos->pointValue();
    } else {
        obj->position = ax::Vec2::ZERO;
    }

    if (auto* vRot = dictionary->valueForKey("rotation")) {
        obj->rotation = vRot->floatValue();
    } else {
        obj->rotation = 0.0f;
    }

    if (auto* vScale = dictionary->valueForKey("scale")) {
        obj->scale = vScale->pointValue();
    } else {
        obj->scale = ax::Vec2(1.0f, 1.0f);
    }

    if (auto* vInfoIdx = dictionary->valueForKey("infoObjIndex")) {
        obj->infoObjIndex = vInfoIdx->intValue();
    } else {
        obj->infoObjIndex = 0;
    }

    obj->infoObjKey = (ax::__String*)dictionary->valueForKey("infoObjKey");
    
    return obj;
    
}
