//
//  PZGGameInfoLevel.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 4/23/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGameInfoLevel.h"
#include "PZGGameInfoLevelItem.h"

PZGGameInfoLevel::PZGGameInfoLevel()
{
    objects = nullptr;
    index = 0;
    startDistance = 0.0f;
    endDistance = 0.0f;
    platformInfoKey = nullptr;
    platformInfoIndex = 0;
    name = nullptr;
    className = nullptr;
    key = nullptr;
}
PZGGameInfoLevel::~PZGGameInfoLevel()
{
    AX_SAFE_RELEASE_NULL(objects);
}

PZGGameInfoLevel*  PZGGameInfoLevel::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameInfoLevel *obj = new PZGGameInfoLevel();
    
    if (!dictionary) {
        AXLOGE("PZGGameInfoLevel::createWithDictionary got null dictionary");
        return obj;
    }

    auto* vName = dictionary->valueForKey("name");
    obj->name = vName ? vName : ax::__String::create("");

    auto* vClass = dictionary->valueForKey("className");
    obj->className = vClass ? vClass : ax::__String::create("");

    auto* vKey = dictionary->valueForKey("key");
    obj->key = vKey ? vKey : ax::__String::create("");

    auto* vIndex = dictionary->valueForKey("index");
    obj->index = vIndex ? vIndex->intValue() : 0;

    auto* vStart = dictionary->valueForKey("startDistance");
    obj->startDistance = vStart ? vStart->floatValue() : 0.0f;

    auto* vEnd = dictionary->valueForKey("endDistance");
    obj->endDistance = vEnd ? vEnd->floatValue() : 0.0f;

    obj->platformInfoKey = (ax::__String*)dictionary->valueForKey("platfromInfoKey");
    auto* vPlatIdx = dictionary->valueForKey("platformInfoIndex");
    obj->platformInfoIndex = vPlatIdx ? vPlatIdx->intValue() : 0;

    
    AX_SAFE_RELEASE_NULL(obj->objects);
    obj->objects = ax::__Array::create();
    // Stored as a member; retain so it survives autorelease pool drains between restarts.
    AX_SAFE_RETAIN(obj->objects);
    
    ax::__Array* array = (ax::__Array*)dictionary->objectForKey("objects");
    if (!array) {
        AXLOGE("PZGGameInfoLevel '{}' missing 'objects' array", obj->key ? obj->key->getCString() : "");
        return obj;
    }

    for (int i=0; i<array->count(); i++) {
        ax::__Dictionary* dict = (ax::__Dictionary*)array->objectAtIndex( i );
        if (!dict) continue;

        PZGGameInfoLevelItem *item = PZGGameInfoLevelItem::createWithDictionary( dict );
        if (item) obj->objects->addObject( item );
    }
    
    
    return obj;
    
}
