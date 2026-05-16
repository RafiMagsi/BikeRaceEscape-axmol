//
//  PZGArtObstacle.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/7/12.
//
//

#ifndef __ProjectMayhem__PZGArtObstacle__
#define __ProjectMayhem__PZGArtObstacle__

#include "PZGArtObject.h"

#define C_ART_OBSTACLE_CLASS_NAME "PZArtObstacle"

class PZGArtObstacle : public PZGArtObject
{
public:
    
    PZGArtObstacle();
    ~PZGArtObstacle();
    
    float comingRatio;
    float health;
    float baseSpeed;
    float scale;
    
    bool destroyCharacter;
    bool collideCharacter;
    
    bool randomizeScale;
    bool randomizeRotation;
    bool animateRotation;
    bool bouncingAnimation;
    
    int trajectory;
 
    ax::Vec2 collision[ 5 ];
    ax::Vec2 offset;
    
    PZGArtObject *deathArtObj;
    
    static PZGArtObstacle* createWithDictionary(ax::__Dictionary * dictionary);
    
};

#endif /* defined(__ProjectMayhem__PZGArtObstacle__) */
