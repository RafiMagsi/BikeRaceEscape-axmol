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
    
}
PZGGameInfoLevel::~PZGGameInfoLevel()
{
    
}

PZGGameInfoLevel*  PZGGameInfoLevel::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameInfoLevel *obj = new PZGGameInfoLevel();
    
    obj->name = dictionary-> valueForKey("name");
    obj->className = dictionary->valueForKey("className");
    obj->key = dictionary->valueForKey("key");
    
    obj->index = dictionary->valueForKey("index")->intValue();
    
    obj->startDistance = dictionary->valueForKey("startDistance")->floatValue();
    obj->endDistance = dictionary->valueForKey("endDistance")->floatValue();
    
    obj->platformInfoKey = (ax::__String*)dictionary->valueForKey("platfromInfoKey");
    if (obj->platformInfoKey) {
        obj->platformInfoKey->retain();
    }
    obj->platformInfoIndex = dictionary->valueForKey("platformInfoIndex")->intValue();

    
    obj->objects = ax::__Array::create();
    obj->objects->retain();
    
    ax::__Array* array = (ax::__Array*)dictionary->objectForKey("objects");
    
    for (int i=0; i<array->count(); i++) {
        ax::__Dictionary* dict = (ax::__Dictionary*)array->objectAtIndex( i );
        
        PZGGameInfoLevelItem *item = PZGGameInfoLevelItem::createWithDictionary( dict );
        obj->objects->addObject( item );
    }
    
    
    return obj;
    
}
