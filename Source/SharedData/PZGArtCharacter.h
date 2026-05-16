//
//  PZGArtCharacter.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 10/30/12.
//
//

#ifndef __ProjectMayhem__PZGArtCharacter__
#define __ProjectMayhem__PZGArtCharacter__

#include "PZGArtObject.h"
#include "Contants_data.h"

#define C_ART_CHARACTER_CLASS_NAME "PZArtCharacter"
#define C_ART_CHARACTER_BOUNCING "bouncing"
#define C_ART_CHARACTER_SPEEDSCALE_X "speed scale X"
#define C_ART_CHARACTER_SPEEDSCALE_Y "speed scale Y"
#define C_ART_CHARACTER_NAME "name"
#define C_ART_CHARACTER_DESCRIPTION "description"
#define C_ART_CHARACTER_PRICE "price"

class PZGArtCharacter : public PZGArtObject
{
public:
    
    
    PZGArtCharacter();
    ~PZGArtCharacter();
    
    float scale;
    
    int price;
    bool bouncing;

    bool enableIAP;
    ax::__String *storeId;
    ax::__String *storeDescription;
    
    ax::Vec2 collision[ 5 ];
    
    const ax::__String *desctiprion;

    
    PZGArtObject *deathArtObj;
    PZGArtObject *fireArtObj;
    PZGArtObject *bulletArtObj;
    PZGArtObject *iconArtObj;
    PZGArtObject *jumpArtObj;
    
    static PZGArtCharacter* createWithDictionary(ax::__Dictionary * dictionary);
    
};

#endif /* defined(__ProjectMayhem__PZGArtCharacter__) */
