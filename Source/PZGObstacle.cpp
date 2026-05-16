//
//  PZGObstacle.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/10/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGObstacle.h"
#include "constants.h"
#include "PZSettingsController.h"

USING_NS_AX;

PZGObstacle* PZGObstacle::create( PZGArtObstacle* obstInfo, b2World *world){
    
    PZGObstacle * obstacle = new PZGObstacle();
    
    obstacle->init( obstInfo );
    
    if (obstacle->sprite == NULL) {
        return NULL;
    }
    
    if (obstInfo->destroyCharacter || obstInfo->collideCharacter) {
        obstacle->initPhysics( obstInfo, world );
    }

    obstacle->setTag( kObstacleTag );

    if (obstInfo->randomizeRotation) {
        obstacle->setRotation(CCRANDOM_0_1()*360);
    }
    
    return (PZGObstacle*)obstacle;
}

void PZGObstacle::init(PZGArtObstacle* obstInfo){

    if (obstInfo->randomizeScale) {
        obstacle_scale = CCRANDOM_0_1() * 0.6 + 0.7;
    }
    else{
        obstacle_scale = obstInfo->scale;
    }

    bouncingAnimation = obstInfo->bouncingAnimation;
    rotationAnimation = obstInfo->animateRotation;
    
    if (rotationAnimation) {
        rotationVelocity = CCRANDOM_MINUS1_1();
    }
    
    trajectory = (PZGObstacleTrajectory)obstInfo->trajectory;
    
    sprite = obstInfo->getResource();
    if (sprite) {
        sprite->setPosition( ccp(obstInfo->offset.x * kDeviceScale()*2, obstInfo->offset.y * kDeviceScale()*2)  );
        sprite->setAnchorPoint( ccp(0.5, 0.5) );
        sprite->setScale( obstacle_scale );
        this->addChild(sprite);
    }

    
    sprite_flash = obstInfo->getResource();
    if (sprite_flash) {
        sprite_flash->setAnchorPoint( ccp(0.5, 0.5) );
        sprite_flash->setBlendFunc((BlendFunc){GL_SRC_ALPHA, GL_ONE});
        sprite_flash->setScale( obstacle_scale );
        
        
        this->addChild(sprite_flash);
    }
    
    sprite_death = obstInfo->deathArtObj->getResourceAnimate();
    if (sprite_death) {
        sprite_death->retain();
    }
    
    health = obstInfo->health;
    speed = obstInfo->baseSpeed;
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        int r = rand() % kMaxObstacleDeathSounds;
        int soundID = kSoundIDObstacleDeath1;
        switch (r) {
            case 0: soundID = kSoundIDObstacleDeath1; break;
            case 1: soundID = kSoundIDObstacleDeath2; break;
            case 2: soundID = kSoundIDObstacleDeath3; break;
            default:
                break;
        }
        deathSound = (PZGSoundData*)sounds->objectAtIndex( soundID );
    }

    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral * gameInfoGeneral = (PZGGameInfoGeneral*)array->objectAtIndex(0);
    Size screen = Director::getInstance()->getWinSize();
    PZGObstacleTrajectory trj;
    if (trajectory == 9 ) {
        trj = (PZGObstacleTrajectory)(rand() % 7 + 1);
    }
    else{
        trj = trajectory;
    }

    if ( gameInfoGeneral->isPortraitMode == false) {
        switch ( trj ) {
            case PZGObstacleTrajectoryLeftLinear:
                setPosition( ccp( screen.width*1.2 , CCRANDOM_0_1() * screen.height) );
                velocityLinear = ccp(1.0, 0.0);
                break;
                
            case PZGObstacleTrajectoryLeftDiagonalDown:
                setPosition( ccp(screen.width*1.2, CCRANDOM_0_1() * screen.height/2.0 + screen.height/2.0 ));
                velocityLinear = ccp(0.9, 0.35);
                break;
                
            case PZGObstacleTrajectoryLeftDiagonalUp:
                setPosition( ccp(screen.width*1.2, CCRANDOM_0_1() * screen.height/2.0 ));
                velocityLinear = ccp(0.9, -0.35);
                break;
                
            case PZGObstacleTrajectoryDown:
                setPosition( ccp(CCRANDOM_0_1() * screen.width, screen.height*1.2 ));
                velocityLinear = ccp(0.2, 0.8);
                break;
            case PZGObstacleTrajectoryUp:
                setPosition( ccp(CCRANDOM_0_1() * screen.width, -screen.height*0.2 ));
                velocityLinear = ccp(0.2, -0.8);
                break;
                
            case PZGObstacleTrajectoryRightLinear:
                setPosition( ccp( -screen.width*0.2 , CCRANDOM_0_1() * screen.height) );
                velocityLinear = ccp(-1.0, 0.0);
                break;
                
            case PZGObstacleTrajectoryRightDiagonalDown:
                setPosition( ccp(-screen.width*0.2, CCRANDOM_0_1() * screen.height/2.0 + screen.height/2.0 ));
                velocityLinear = ccp(-0.9, 0.35);
                break;
                
            case PZGObstacleTrajectoryRightDiagonalUp:
                setPosition( ccp(-screen.width*0.2, CCRANDOM_0_1() * screen.height/2.0 ));
                velocityLinear = ccp(-0.9, -0.35);
                break;
                
            default:
                break;
        }
    }
    else{
        switch ( trj ) {
            case PZGObstacleTrajectoryLeftLinear:
                setPosition( ccp( screen.width*1.2, CCRANDOM_0_1() * screen.height ) );
                velocityLinear = ccp(1.0, 0.1);
                break;
                
            case PZGObstacleTrajectoryLeftDiagonalDown:
                //setPosition( ccp(screen.width*1.2, (CCRANDOM_0_1() * screen.height/2.0) + screen.height/2.0 ));
                setPosition( ccp( CCRANDOM_0_1() * screen.width/2.0 + screen.width/2.0,screen.height*1.2));
                velocityLinear = ccp(0.35, 0.9);
                break;
                
            case PZGObstacleTrajectoryLeftDiagonalUp:
                //setPosition( ccp(screen.width*1.2, CCRANDOM_0_1() * screen.height/2.0 ));
                setPosition( ccp( CCRANDOM_0_1() * screen.width/2.0+ screen.width/2.0, -screen.height*0.2 ));
                velocityLinear = ccp(0.4, -0.9);
                break;
                
            case PZGObstacleTrajectoryDown:
                setPosition( ccp(CCRANDOM_0_1() * screen.width, screen.height*1.2 ));
                velocityLinear = ccp(0.0, 1.0);
                break;
                
            case PZGObstacleTrajectoryUp:
                setPosition( ccp(CCRANDOM_0_1() * screen.width, -screen.height*0.2 ));
                velocityLinear = ccp(0.0, -1.0);
                break;
                //////
            case PZGObstacleTrajectoryRightLinear:
                setPosition( ccp( -screen.width*0.2, CCRANDOM_0_1() * screen.height) );
                velocityLinear = ccp(-1.0, 0.1);
                break;
                
            case PZGObstacleTrajectoryRightDiagonalDown:
                setPosition( ccp(-screen.width*0.2, CCRANDOM_0_1() * screen.height/2.0 + screen.height/2.0 ));
                velocityLinear = ccp(-0.9, 0.4);
                break;
                
            case PZGObstacleTrajectoryRightDiagonalUp:
               //setPosition( ccp(-screen.width*0.2, CCRANDOM_0_1() * screen.height/2.0 ));
                setPosition( ccp( CCRANDOM_0_1() * screen.width/2.0, -screen.height*0.2 ));
                velocityLinear = ccp(-0.4, -0.9);
                break;

            default:
                break;

        }
    }
}

void PZGObstacle::setRotation(float angle){
    if (sprite) {
        sprite->setRotation(angle);
    }

    if (sprite_flash) {
        sprite_flash->setRotation(angle);
    }

    if (body) {
        b2Vec2 pos = body->GetPosition();
        body->SetTransform(pos, CC_DEGREES_TO_RADIANS(-angle));
    }
}

void PZGObstacle::setScaleX(float scale){
    Node::setScaleX( scale );
    
    if (body) {
        b2Fixture* fixture = body->GetFixtureList();
        b2PolygonShape* shape = (b2PolygonShape*)fixture->GetShape();

        b2Vec2 vtx[ 8 ];
        for (int i=0; i < shape->GetVertexCount(); i++) {
            vtx[ i ] = shape->GetVertex( i );
            vtx[ i ] = b2Vec2(vtx[ i ].x * scale, vtx[ i ].y);
        }
        shape->Set(vtx, shape->GetVertexCount());
    }
}

void PZGObstacle::setScaleY(float scale){
    Node::setScaleY( scale );
    
    if (body) {
        b2Fixture* fixture = body->GetFixtureList();
        b2PolygonShape* shape = (b2PolygonShape*)fixture->GetShape();
        
        b2Vec2 vtx[ 8 ];
        for (int i=0; i < shape->GetVertexCount(); i++) {
            vtx[ i ] = shape->GetVertex( i );
            vtx[ i ] = b2Vec2(vtx[ i ].x, vtx[ i ].y * scale);
        }
        shape->Set(vtx, shape->GetVertexCount());
    }

}

void PZGObstacle::initPhysics( PZGArtObstacle* obstInfo, b2World *world ){
    
    if (sprite == NULL) {
        return;
    }
    
    destroyCharacter = obstInfo->destroyCharacter;
    
    Vec2 pp = ccp(-500, 700) ;
    b2BodyDef playerBodyDef;
    playerBodyDef.type = b2_staticBody;
    playerBodyDef.fixedRotation = true;
    playerBodyDef.position.Set(pp.x*kPhysicsWorldScale(), pp.y*kPhysicsWorldScale());
    
    body = world->CreateBody( &playerBodyDef );
    
    
    b2PolygonShape shape;
    
    int const vtx_count = 5;
    b2Vec2 vtx[ vtx_count ];
    
    bool zeroCollisonPoints = true;
    
    for (int i=0; i < vtx_count; i++) {
        if ( ccpLength( obstInfo->collision[ i ] ) >= 0.1) {
            zeroCollisonPoints = false;
        }
    }
    
    if (zeroCollisonPoints == false) {
        for (int i=0; i < vtx_count; i++) {
            vtx[ i ] = b2Vec2((obstInfo->collision[ i ].x*kDeviceScale() - sprite->getContentSize().width*0.5)*kPhysicsWorldScale() * obstacle_scale,
                              (obstInfo->collision[ i ].y*kDeviceScale() - sprite->getContentSize().height*0.5)*kPhysicsWorldScale() * obstacle_scale);
        }
        shape.Set(vtx, vtx_count);
    }
    else{
        Size size = sprite->getContentSize();
        shape.SetAsBox( size.width*0.5*kPhysicsWorldScale() * obstacle_scale,
                       size.height*0.5*kPhysicsWorldScale() * obstacle_scale);
    }
    
    b2FixtureDef fixture;
    fixture.density = 0.1;
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


void PZGObstacle::update( float dt){
    
    counter += dt;
    if (counter >= M_PI * 10) {
        counter = 0;
    }
    
    Vec2 loc = this->getPosition();
    
    if (velocityLinear.x != 0) {
        loc.x = loc.x  - (( gameSpeed * (1+speed) ) * velocityLinear.x);
    }
    
    if (velocityLinear.y != 0) {
        loc.y = loc.y  - (( gameSpeed * (1+speed) ) * velocityLinear.y);
    }
    
    this->setPosition(loc);
    
    
//    printf("pos : %f,%f\n",loc.x,loc.y);
    
    if (body && sprite) {
        
        float angle= body->GetAngle();
        body->SetTransform(b2Vec2((this->getPosition().x + sprite->getPosition().x) * kPhysicsWorldScale(),
                                  (this->getPosition().y + sprite->getPosition().y) * kPhysicsWorldScale()), angle);        

        if (bouncingAnimation) {
            sprite->setScaleX( (sinf(counter*20) + 1)/2.0 * 0.08 + obstacle_scale);
            sprite->setScaleY( (cosf(counter*20) + 1)/2.0 * 0.08 + obstacle_scale);
        }
    }
    
//    if (velocityLinear.x > 0) {
//        this->setPosition(this->getPosition().x  - ( gameSpeed * (1+speed) ) * velocityLinear.x,
//                          this->getPosition().y  - ( gameSpeed * (1+speed) ) * velocityLinear.y);
//    }
//    else{
//        this->setPosition(this->getPosition().x  - ( gameSpeed * (1+speed) ) * velocityLinear.x,
//                          this->getPosition().y  - ( gameSpeed * (1+speed) ) * velocityLinear.y);
//    }
    
    
    if (sprite_flash) {
        sprite_flash->setOpacity( clampf(damage_counter*255, 0, 255) );
        sprite_flash->setPosition( sprite->getPosition() );
        sprite_flash->setRotation( sprite->getRotation() );
        sprite_flash->setScaleX( sprite->getScaleX() );
        sprite_flash->setScaleY( sprite->getScaleY() );
    }
    
    if(rotationAnimation){
        if (sprite) {
            setRotation( sprite->getRotation() + rotationVelocity*dt*32 );
        }
    }
    
    
    if (damage_counter > 0.0) {
        damage_counter -= dt*3;
    }
    else{
        damage_counter = 0;
    }
    
    if (dead && deleteReady == false) {
        if (sprite_death) {
            if (sprite_death->isDone()) {
                deleteReady = true;
            }
        }
    }
}


void PZGObstacle::damage(float power){
    if (dead) {
        return;
    }
    health -= power;
    
    if(health <= 0){
        this->removeChild(sprite_flash, true);
        sprite_flash = NULL;
        dead = true;
        
        if (deathSound) {
            deathSound->playAsSound( false );
        }

        
        if (sprite_death) {
            sprite->stopAllActions();
            sprite_death->setTag( 99 );
            sprite->runAction( sprite_death );
        }
        else{
            sprite->removeFromParentAndCleanup( true );
            sprite = NULL;
            
            deleteReady = true;
        }
    }
    else{
        damage_counter = 1.0;
    }
}

void PZGObstacle::destroyPhysics( b2World* world ){
    if (body) {
        world->DestroyBody( body );
        body = NULL;
    }
}
