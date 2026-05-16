//
//  PZGPlatfrom.cpp
//  ProjectMayhem
//
//  Created by Amit Srivastava on 09/05/13.
//
//

#include "PZGPlatfrom.h"
#include "constants.h"

USING_NS_AX;

PZGPlatfrom* PZGPlatfrom::create( PZGArtObject* platformInfo, b2World *world){
    
    PZGPlatfrom * platform = new PZGPlatfrom();
    
    platform->init( platformInfo );
    
    if (platform->sprite == NULL) {
        return NULL;
    }
    
    platform->setTag( kPlatfromTag );
    
    
    return (PZGPlatfrom*)platform;
}

void PZGPlatfrom::init(PZGArtObject* platformInfo){
    
    sprite = platformInfo->getResource();
    if (sprite) {
        this->addChild(sprite);
    }
    
}

void PZGPlatfrom::initPhysics(b2World *world ){
    
    if (sprite == NULL) {
        return;
    }
    
    b2BodyDef playerBodyDef;
    Vec2 spritePosition = sprite->getPosition();
    
    playerBodyDef.type = b2_staticBody;
    playerBodyDef.position.Set(spritePosition.x*kPhysicsWorldScale() + collisionPosition.x*kDeviceScale()*kPhysicsWorldScale(),
                               spritePosition.y*kPhysicsWorldScale() + collisionPosition.y*kDeviceScale()*kPhysicsWorldScale());
    body = world->CreateBody( &playerBodyDef );
    
    b2PolygonShape shape;
    Size ps = sprite->getContentSize();
    shape.SetAsBox(width*kDeviceScale()*kPhysicsWorldScale(), 20*kDeviceScale()*kPhysicsWorldScale());
    
    b2FixtureDef playerFixture;
    playerFixture.density = 4.0;
    playerFixture.friction = 1.0;
    playerFixture.restitution = 0.0;
    playerFixture.shape = &shape;
    playerFixture.userData = this;
    
    body->CreateFixture( &playerFixture );
    
}

void PZGPlatfrom::update( float dt){
    //
}

void PZGPlatfrom::destroyPhysics( b2World* world ){
    if (body) {
        world->DestroyBody( body );
        body = NULL;
    }
}

void PZGPlatfrom::removeFromParentAndCleanup(bool cleanup) {
   
    if (sprite) {
        sprite->removeFromParentAndCleanup(cleanup);
    }
    
    Node::removeFromParentAndCleanup( cleanup );    
}
