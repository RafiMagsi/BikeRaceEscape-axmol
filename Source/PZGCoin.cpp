//
//  PZGCoin.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/10/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGCoin.h"
#include "constants.h"
#include "PZSettingsController.h"

USING_NS_AX;

PZGCoin* PZGCoin::create( PZGArtCoins* obstInfo, b2World *world){

    PZGCoin * coin = new PZGCoin();

    coin->init( obstInfo );
    if (coin->sprite == NULL) {
        return NULL;
    }
    coin->initPhysics( obstInfo, world );
    coin->setTag( kCoinTag );
    
    return coin;
}


void PZGCoin::update( float dt){
    if (body) {
        body->SetTransform(b2Vec2((this->getPosition().x + sprite->getPosition().x) * kPhysicsWorldScale(),
                                  (this->getPosition().y + sprite->getPosition().y) * kPhysicsWorldScale()), 0);
    }
}

void PZGCoin::init(PZGArtCoins* obstInfo){
    scale = obstInfo->scale;

    sprite = obstInfo->getResource();
    if (sprite) {
        sprite->setPosition( ccp(obstInfo->offset.x * kDeviceScale(), obstInfo->offset.y * kDeviceScale())  );
        
        sprite->setAnchorPoint( ccp(0, 0) );
        sprite->setScale(scale);
        reward = obstInfo->reward;
        comingRatio = obstInfo->comingRatio;
        
        this->addChild(sprite);        
    }
}

void PZGCoin::initPhysics( PZGArtCoins* obstInfo, b2World *world ){
    
    if (sprite == NULL) {
        return;
    }
    
    Vec2 pp = ccp(-100,-100) ;
    b2BodyDef playerBodyDef;
    playerBodyDef.type = b2_staticBody;
    playerBodyDef.position.Set(pp.x*kPhysicsWorldScale(), pp.y*kPhysicsWorldScale());
    body = world->CreateBody( &playerBodyDef );
    
    b2PolygonShape shape;
    shape.SetAsBox(sprite->getContentSize().width*kPhysicsWorldScale()*0.5*scale,
                   sprite->getContentSize().height*kPhysicsWorldScale()*0.5*scale,
                   b2Vec2(sprite->getContentSize().width*kPhysicsWorldScale()*0.5*scale, sprite->getContentSize().height*kPhysicsWorldScale()*0.5*scale),
                   0.0f);

    
    b2FixtureDef fixture;
    fixture.density = 0.1;
    fixture.isSensor = true;
    fixture.friction = 1.0;
    fixture.restitution = 0.0;
    fixture.shape = &shape;
    fixture.userData = this;
    PZSettingsController *sc = PZSettingsController::shared();
    if (sc->kidMode) {
        fixture.filter.groupIndex = 2;
    }
    body->CreateFixture( &fixture );
    
}

void PZGCoin::destroyPhysics( b2World* world ){
    if (body) {
        world->DestroyBody( body );
        body = NULL;
    }
}
