//
//  PZGGameInfoLevel.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 4/23/13.
//
//

#ifndef __ProjectMayhem__PZGGameInfoLevel__
#define __ProjectMayhem__PZGGameInfoLevel__

#include <iostream>
#include "PZLegacyCompat.h"

#define C_GAMEINFO_LEVEL_CLASS_NAME "PZGameInfoLevel"

class PZGGameInfoLevel : public ax::Ref
{
public:
    
    PZGGameInfoLevel();
    ~PZGGameInfoLevel();

    ax::__Array* objects;
    
    int index;
    
    float startDistance;
    float endDistance;
    
    ax::__String*    platformInfoKey;
    int                   platformInfoIndex;
    
    const ax::__String * name;
    const ax::__String * className;
    const ax::__String * key;

    static PZGGameInfoLevel* createWithDictionary(ax::__Dictionary * dictionary);
    
};
#endif /* defined(__ProjectMayhem__PZGGameInfoLevel__) */
