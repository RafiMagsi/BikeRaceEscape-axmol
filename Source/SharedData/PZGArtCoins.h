//
//  PZArtCoins.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/7/12.
//
//

#ifndef __ProjectMayhem__PZArtCoins__
#define __ProjectMayhem__PZArtCoins__

#include "PZGArtObject.h"

#define C_ART_COINS_CLASS_NAME "PZArtCoins"

class PZGArtCoins : public PZGArtObject
{
public:
    
    PZGArtCoins();
    ~PZGArtCoins();
    
    float comingRatio;
    int reward;
    float scale;

    ax::Vec2 offset;
    
    static PZGArtCoins* createWithDictionary(ax::__Dictionary * dictionary);
    
};


#endif /* defined(__ProjectMayhem__PZArtCoins__) */
