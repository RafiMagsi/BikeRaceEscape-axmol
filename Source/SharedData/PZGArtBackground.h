//
//  PZGArtBackground.h
//  ProjectZero
//
//  Created by Nik Rudenko on 10/10/12.
//
//

#ifndef __ProjectZero__PZGArtBackground__
#define __ProjectZero__PZGArtBackground__

#include <iostream>
#include "PZGArtObject.h"
#include "Contants_data.h"

#define C_ART_BACKGROUND_CLASS_NAME "PZArtBackground"
#define C_ART_BG_SPEED "speed"
#define C_ART_BG_POSITION "position"
#define C_ART_BG_SCALE "scale"

class PZGArtBackground : public PZGArtObject
{
public:

    PZGArtBackground();
    ~PZGArtBackground();
    
    float speed;
    float scale;
    float position;
    float opacity;
    
    bool playGroundLayer;
    
    static PZGArtBackground* createWithDictionary(ax::__Dictionary * dictionary);
    
};


#endif /* defined(__ProjectZero__PZGArtBackground__) */
