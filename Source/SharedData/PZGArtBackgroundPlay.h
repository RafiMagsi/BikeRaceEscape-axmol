//
//  PZGArtBackgroundPlay.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 3/15/13.
//
//

#ifndef __ProjectMayhem__PZGArtBackgroundPlay__
#define __ProjectMayhem__PZGArtBackgroundPlay__

#include <iostream>
#include "PZGArtObject.h"
#include "PZGArtBackground.h"
#include "Contants_data.h"

#define C_ART_BACKGROUNDPLAY_CLASS_NAME "PZArtBackgroundPlay"

class PZGArtBackgroundPlay : public PZGArtBackground
{
public:
    
    PZGArtBackgroundPlay();
    ~PZGArtBackgroundPlay();
    
    float boundDown;
    float boundLeft;
    float boundUp;
    float boundRight;
    
    
    static PZGArtBackgroundPlay* createWithDictionary(ax::__Dictionary * dictionary);
    
};

#endif /* defined(__ProjectMayhem__PZGArtBackgroundPlay__) */
