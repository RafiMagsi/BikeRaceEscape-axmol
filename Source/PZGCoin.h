//
//  PZGCoin.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/10/12.
//
//

#ifndef __ProjectMayhem__PZGCoin__
#define __ProjectMayhem__PZGCoin__

#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"
#include "SharedData/PZGArtCoins.h"

#define kCoinTag 201

class PZGCoin : public ax::Node
{
    
    b2Body*             body;
public:

    ax::Sprite*  sprite;

    float   reward;
    float   comingRatio;
    bool    disabled;
    float   scale;
    bool    deleteReady;
    
    static PZGCoin* create( PZGArtCoins* obstInfo, b2World *world );
    void init(PZGArtCoins* obstInfo);
    void initPhysics( PZGArtCoins* obstInfo, b2World *world);
    void update( float dt);
    void destroyPhysics( b2World* world );
    
};


#endif /* defined(__ProjectMayhem__PZGCoin__) */
