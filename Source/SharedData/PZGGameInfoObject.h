//
//  PZGGameInoObject.h
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ProjectZero_PZGGameInfoObject_h
#define ProjectZero_PZGGameInfoObject_h

#include <string>
#include "PZLegacyCompat.h"

class PZGGameInfoObject : public ax::Object
{
    public:
    
    PZGGameInfoObject();
    ~PZGGameInfoObject();

    
    float gameSpeedMin;
    float gameSpeedMax;
    float gameSpeedIncreaser;
    
    
    const ax::__String * name;
    const ax::__String * className;
    const ax::__String * key;
    
    int gameTypeId;
    

    
};

#endif
