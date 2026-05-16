//
//  PZGArtInterface.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/13/12.
//
//

#ifndef __ProjectMayhem__PZGArtInterface__
#define __ProjectMayhem__PZGArtInterface__

#include "PZGArtObject.h"

#define C_ART_INTERFACE_CLASS_NAME "PZArtInterface"


enum PZGInterfaceAlign {
    PZGInterfaceAlignNone = 0,
    PZGInterfaceAlignUpLeft = 1,
    PZGInterfaceAlignUp =  2,
    PZGInterfaceAlignUpRight = 3,
    PZGInterfaceAlignLeft = 4,
    PZGInterfaceAlignCenter = 5,
    PZGInterfaceAlignRight = 6,
    PZGInterfaceAlignDownLeft = 7,
    PZGInterfaceAlignDown = 8,
    PZGInterfaceAlignDownRight = 9,
};


class PZGArtInterface : public PZGArtObject
{
public:
    
    PZGArtInterface();
    ~PZGArtInterface();
    
    bool bouncing;
    bool position_lock;
    
    float position_x;
    float position_y;
    
    float scale_x;
    float scale_y;
    
    float rotation;
    
    PZGInterfaceAlign alignment;
    
    ax::__String* type;
    ax::__String* subkey;
    
    ax::Vec2 getPosition();
    
    static PZGArtInterface* createWithDictionary(ax::__Dictionary * dictionary);
    
};

#endif /* defined(__ProjectMayhem__PZGArtInterface__) */
