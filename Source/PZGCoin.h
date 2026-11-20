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
    
    b2Body*             body = nullptr;
public:

    ax::Sprite*  sprite = nullptr;

    float   reward = 1.f;
    float   comingRatio = 0.f;
    bool    disabled = false;
    float   scale = 1.f;
    bool    deleteReady = false;
    
    static PZGCoin* create( PZGArtCoins* obstInfo, b2World *world );
    void init(PZGArtCoins* obstInfo);
    void initPhysics( PZGArtCoins* obstInfo, b2World *world);
    void update( float dt);
    void destroyPhysics( b2World* world );
    
};


#endif /* defined(__ProjectMayhem__PZGCoin__) */
