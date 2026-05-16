//
//  PZArtObject_cpp.h
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ProjectZero_PZGArtObject_h
#define ProjectZero_PZGArtObject_h

#include <string>
#include "PZLegacyCompat.h"

#define kDefaultAnimationTag 658

class PZGArtObject : public ax::Object
{
public:
    PZGArtObject();
    ~PZGArtObject();
    static PZGArtObject* createWithDictionary(ax::__Dictionary * dictionary);
    
    const ax::__String * objectType;
    const ax::__String * className;
    
    ax::__String * name;
    ax::__String * key;

    float animationSpeed;
    
    int no_of_sprites;
    int index;
        
    ax::Sprite*  getResource();
    ax::Animate* getResourceAnimate();
    ax::__Array*   getResourceFrameNames();

    ax::__Dictionary* getDataInDictionary();
};

#endif
