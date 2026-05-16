//
//  PZGGameInfoLevelItem.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 4/23/13.
//
//

#ifndef __ProjectMayhem__PZGGameInfoLevelItem__
#define __ProjectMayhem__PZGGameInfoLevelItem__

#include <iostream>
#include "PZLegacyCompat.h"

class PZGGameInfoLevelItem : public ax::Object
{
public:
    
    PZGGameInfoLevelItem();
    ~PZGGameInfoLevelItem();
    
    const ax::__String * name;
    const ax::__String * className;
    const ax::__String * key;
    
    ax::Vec2 position;
    ax::Vec2 scale;
    float rotation;
    
    int                      infoObjIndex;
    ax::__String* infoObjKey;
    
    static PZGGameInfoLevelItem* createWithDictionary(ax::__Dictionary * dictionary);
};

#endif /* defined(__ProjectMayhem__PZGGameInfoLevelItem__) */
