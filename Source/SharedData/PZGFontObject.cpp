//
//  PZGFontObject.cpp
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "PZLegacyCompat.h"
#include <iostream>
#include "PZGFontObject.h"

#include "Contants_data.h"
#include "constants.h"
PZGFontObject::PZGFontObject()
{
    
}
PZGFontObject::~PZGFontObject()
{
    
}

PZGFontObject * PZGFontObject::initWithDictionary(ax::__Dictionary* dictionary)
{
    fontType = dictionary->valueForKey(FONT_TYPE);
    fileName = dictionary->valueForKey(FONT_OBJECT_FILENAME);
    otherInfo = dictionary->valueForKey(FONT_OTHER_INFO);
    className = dictionary->valueForKey(FONT_OBJECT);
    return this;
}
ax::__Dictionary* PZGFontObject::getDataInDictionary()
{
    ax::__Dictionary * tempDict = ax::__Dictionary::create();
    tempDict->setObject((ax::Ref*)fontType, FONT_TYPE);
    tempDict->setObject((ax::Ref*)fileName, FONT_OBJECT_FILENAME);
    tempDict->setObject((ax::Ref*)otherInfo, FONT_OTHER_INFO);
    tempDict->setObject((ax::Ref*)className, FONT_OBJECT);
    return tempDict;
}
