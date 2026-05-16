//
//  PZGPlatfrom.h
//  ProjectMayhem
//
//  Created by Amit Srivastava on 09/05/13.
//
//

#ifndef __ProjectMayhem__PZGPlatfrom__
#define __ProjectMayhem__PZGPlatfrom__

#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"
#include "SharedData/PZGArtObject.h"

#define kPlatfromTag 75

class PZGPlatfrom : public ax::Node
{
    
    public:
    
    b2Body*             body;
    ax::Sprite*  sprite;
    float width;
    ax::Vec2 collisionPosition;
    
    static PZGPlatfrom* create( PZGArtObject* platformInfo, b2World *world );
    void initPhysics(b2World *world );
    void init(PZGArtObject* platformInfo);
    void update( float dt);
    void destroyPhysics( b2World* world );
    
    virtual void removeFromParentAndCleanup(bool cleanup);
};

#endif /* defined(__ProjectMayhem__PZGPlatfrom__) */
