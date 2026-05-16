//
//  PZGFontObject.h
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ProjectZero_PZGFontObject_h
#define ProjectZero_PZGFontObject_h
#include <string>
#include <map>
#include "PZLegacyCompat.h"
class PZGFontObject : public ax::Object
{
public:
   const ax::__String * fontType;
   const ax::__String * fileName;
   const ax::__String * otherInfo;
   const ax::__String * className;
    
    PZGFontObject();
    ~PZGFontObject();
    
    PZGFontObject * initWithDictionary(ax::__Dictionary* dictionary);
    ax::__Dictionary* getDataInDictionary();
   // ax::ccla
    void setResourceAtPath(ax::__String* path);
    
//    PZGFontObject * getThisObject();
//    ax::__String *getFontResourcePath();
    
};
#endif
