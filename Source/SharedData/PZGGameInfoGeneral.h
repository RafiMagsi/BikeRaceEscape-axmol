//
//  PZGameInfoGeneral.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/3/13.
//
//

#ifndef __ProjectMayhem__PZGGameInfoGeneral__
#define __ProjectMayhem__PZGGameInfoGeneral__

#include <iostream>
#include "PZLegacyCompat.h"

#define C_GAMEINFO_GENERAL_CLASS_NAME "PZGameInfoGeneral"

class PZGGameInfoGeneral : public ax::Object
{
public:
    
    PZGGameInfoGeneral();
    ~PZGGameInfoGeneral();
    
    
    bool isPortraitMode;
    bool isDebugMode;
    
    const ax::__String * name;
    const ax::__String * className;
    const ax::__String * key;
    ax::__String* neocortexURLAndroid;
    ax::__String* neocortexURL;
    
    ax::__String* bundleid;
    
    bool isGameCenterEnabled;
    
    static PZGGameInfoGeneral* createWithDictionary(ax::__Dictionary * dictionary);
    
};
#endif /* defined(__ProjectMayhem__PZGameInfoGeneral__) */
