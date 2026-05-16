//
//  PZGCharacter.cpp
//  ProjectZero
//
//  Created by Nik Rudenko on 8/25/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGCharacter.h"
#include "constants.h"
#include "SharedData/PZGSharedData.h"
#include "SharedData/PZGArtCharacter.h"

#include "PZSettingsController.h"

PZGCharacter* PZGCharacter::create(int playerId, b2World* world, bool gravityAffected ){
    PZGCharacter* player = new PZGCharacter();

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* characters = (__Array*)gsd->artResource->objectForKey( "Characters" );
    if (characters == NULL) {
        return NULL;
    }
    if (playerId >= characters->count()) {
        playerId = 0;
    }
    PZGArtCharacter* artCharacter = ( PZGArtCharacter* )characters->objectAtIndex( playerId );

    player->init( artCharacter);
   
    if (player->s_body == NULL) {
        return NULL;
    }
    
    if (world) {
        player->initPhysics( artCharacter, world, gravityAffected);
    }


    player->setTag( kCharacterTag ); 
    return player;
}

void PZGCharacter::reset(){
    if (s_body_animation) {
        s_body->stopAllActions();
        s_body->runAction( CCRepeatForever::create( s_body_animation ) );
    }
    
    jump_nongrounded_timer = 0;
}

void PZGCharacter::init( PZGArtCharacter* artCharacter ){
    if (!artCharacter) {
        AXLOGE("PZGCharacter::init: artCharacter is null");
        return;
    }

    character_scale = artCharacter->scale;

    Size size = Director::getInstance()->getWinSize();

    AXLOGI("PZGCharacter::init: Creating bullets");
    if (artCharacter->bulletArtObj) {
        for (int i=0; i < kPlayerMaxBullets; i++) {
            s_bullet[ i ] = artCharacter->bulletArtObj->getResource();
            if (s_bullet[ i ]) {
                s_bullet[ i ]->setScale(character_scale);
                s_bullet[ i ]->setAnchorPoint( ccp(0.5, 0.5) );
                s_bullet[ i ]->setPosition( ccp( -100, -100 ) );
                s_bullet[ i ]->setOpacity( 0 );
                this->addChild(s_bullet[ i ]);
            }
        }
    } else {
        AXLOGW("PZGCharacter::init: bulletArtObj is null, skipping bullets");
    }

    AXLOGI("PZGCharacter::init: Getting character body");
    s_body = artCharacter->getResource();
    if (s_body) {
        s_body->setAnchorPoint( ccp(0.5, 0.5) );
        s_body->setPosition( ccp( 0.0, 0.0 ) );
        s_body->setScale( character_scale );

        s_body_animation = artCharacter->getResourceAnimate();
        this->addChild( s_body );
        AXLOGI("PZGCharacter::init: Body added");
    } else {
        AXLOGW("PZGCharacter::init: artCharacter->getResource() returned null");
    }

    AXLOGI("PZGCharacter::init: Setting up death animation");
    if (artCharacter->deathArtObj) {
        s_body_death = artCharacter->deathArtObj->getResourceAnimate();
    } else {
        AXLOGW("PZGCharacter::init: deathArtObj is null");
    }

    AXLOGI("PZGCharacter::init: Setting up jump animation");
    if (artCharacter->jumpArtObj) {
        s_body_jump = artCharacter->jumpArtObj->getResourceAnimate();
    } else {
        AXLOGW("PZGCharacter::init: jumpArtObj is null");
    }

    AXLOGI("PZGCharacter::init: Setting up fire animation");
    if (artCharacter->fireArtObj) {
        s_fire = artCharacter->fireArtObj->getResource();
        if (s_fire) {
            s_fire->setAnchorPoint( ccp(0.5, 0.5) );
            s_fire->setPosition( ccp( 50, -10 ) );
            s_fire->setScale( character_scale );
            this->addChild(s_fire);
        }
    } else {
        AXLOGW("PZGCharacter::init: fireArtObj is null");
    }
    
    
    //SETTINGS
    bouncing = artCharacter->bouncing;
    speed = ccp( 1, 1);
    
    bullet_speed = 1.0;
    fire_speed = 1.0;
    
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        shootingSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDShootingSound );
        shootingSound->preloadSound();
    }
    
    jump_nongrounded_timer = 0;
}

void PZGCharacter::initPhysics( PZGArtCharacter* obstInfo, b2World* world, bool gravityAffected){
    
    if (s_body == NULL) {
        return;
    }
    
    Vec2 pp = this->getPosition();
    b2BodyDef playerBodyDef;
    
    playerBodyDef.type = b2_dynamicBody;
    
    playerBodyDef.fixedRotation = true;
    playerBodyDef.position.Set(pp.x*kPhysicsWorldScale(), pp.y*kPhysicsWorldScale());
    p_body = world->CreateBody( &playerBodyDef );
    
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
            vtx[ i ] = b2Vec2((obstInfo->collision[ i ].x*kDeviceScale() - s_body->getContentSize().width*0.5)*kPhysicsWorldScale() * character_scale,
                              (obstInfo->collision[ i ].y*kDeviceScale() - s_body->getContentSize().height*0.5)*kPhysicsWorldScale() * character_scale);
        }
        shape.Set(vtx, vtx_count);
    }
    else{
        Size size = s_body->getContentSize();
        shape.SetAsBox( size.width*0.5*kPhysicsWorldScale()*character_scale, size.height*0.5*kPhysicsWorldScale()*character_scale);
    }
    
    b2FixtureDef playerFixture;
    playerFixture.density = 0.1;
    playerFixture.friction = 1.0;
    playerFixture.restitution = 0.0;
    playerFixture.shape = &shape;
    playerFixture.userData = this;

    p_body->CreateFixture( &playerFixture );
    p_body->SetSleepingAllowed( false );
    
    //BULLET PHYSICS
    for (int i=0; i < kPlayerMaxBullets; i++) {
        if (s_bullet[ i ]) {
            b2BodyDef bulletBodyDef;
            bulletBodyDef.type = b2_dynamicBody;
            bulletBodyDef.position.Set(+100, -30);
            bulletBodyDef.bullet = true;
            p_bullet[ i ] = world->CreateBody( &bulletBodyDef );
            
            b2PolygonShape bulletShape;
            bulletShape.SetAsBox(s_bullet[ i ]->getContentSize().width*0.5*character_scale*kPhysicsWorldScale(),
                                 s_bullet[ i ]->getContentSize().height*0.5*character_scale*kPhysicsWorldScale());
            
            b2FixtureDef bulletFixture;
            bulletFixture.shape = &bulletShape;
            bulletFixture.userData = this;
            
            p_bullet[ i ]->CreateFixture( &bulletFixture );
            p_bullet[ i ]->SetSleepingAllowed( false );
        }
    }
}

void PZGCharacter::setLoc( Vec2 _loc){
    if (p_body) {
        p_body->SetTransform(b2Vec2(_loc.x*kPhysicsWorldScale(), _loc.y*kPhysicsWorldScale()), p_body->GetAngle());
    }

    loc = _loc;
}
void PZGCharacter::setAngle( float _angle){
    if (p_body) {
        b2Vec2 pos = p_body->GetPosition();
        p_body->SetTransform(pos, - CC_DEGREES_TO_RADIANS( _angle ));
    }

    angle = _angle;    
}

void PZGCharacter::setShootingAngle(float _shooting_angle){
    shooting_angle = _shooting_angle;
    if (s_fire) {
        s_fire->setRotation( shooting_angle );
    }
}

void PZGCharacter::dead(){
    if (s_body_death) {
        s_body->stopAllActions();
        s_body_death->setTag( 99 );
        s_body->runAction( s_body_death );
    }
}


void PZGCharacter::jump_grounded(){
    jump_counter = 0;
    jump_nongrounded = false;
    jump_nongrounded_timer = 0;
    
    if (jump_permanently) {
        jump();
    }
    else{
        if (s_body_jump && s_body_animation) {
            s_body->stopAllActions();
            s_body->runAction( CCRepeatForever::create( s_body_animation ) );
        }
    }
}


void PZGCharacter::jump(){
    if (jump_counter < 1) {
        p_body->SetLinearVelocity(b2Vec2(0, jump_height)); //35 is default value
        jump_counter += 1;
        jump_timer = 0.0;
        jump_enabled = true;
        
        if (s_body_jump) {
            s_body->stopAllActions();
            s_body_jump->setTag( 98 );
            s_body->runAction( s_body_jump );
        }
    }
}

void PZGCharacter::jumpEnd(){
    if (jump_timer < 0.75 && jump_enabled == true) {
        b2Vec2 vel = p_body->GetLinearVelocity();
        float ratio  = jump_timer / 0.75;
        p_body->SetLinearVelocity( b2Vec2(vel.x, vel.y*ratio) );
    }
    jump_enabled = false;

}

void PZGCharacter::update(float dd){
    counter += dd*fire_speed;
    
    if (jump_nongrounded ) {
        jump_nongrounded_timer += dd;
        if (jump_nongrounded_timer >= 0.2) {
            jump_nongrounded = false;
            jump_counter = 1;
            jump_nongrounded_timer = 0;
        }
    }
    
    bouncing_counter += dd;
    
    
    if (jump_enabled) {
        if (jump_timer < 0.75) {
            jump_timer += dd;
        }
        else{
            jumpEnd();
        }
    }
    
    if (p_body) {
        b2Vec2 pp = p_body->GetPosition();
        loc = ccp(pp.x*1/kPhysicsWorldScale(), pp.y*1/kPhysicsWorldScale());
    }
    
    s_body->setRotation( angle );
    s_body->setPosition( loc );

    if (bouncing) {
        s_body->setScaleX( (sinf(bouncing_counter*20) + 1)/2.0 * 0.05 + 1);
        s_body->setScaleY( (cosf(bouncing_counter*20) + 1)/2.0 * 0.05 + 1);
    }
    
    Vec2 shootingPointOffset = ccp(sinf(CC_DEGREES_TO_RADIANS( shooting_angle+90 )), cosf(CC_DEGREES_TO_RADIANS( shooting_angle+90 )));
    Vec2 shootingPoint = ccp( s_body->getContentSize().width * shootingPointOffset.x * 0.45 * character_scale,
                                 s_body->getContentSize().height * shootingPointOffset.y * 0.45 * character_scale);
    
    if(shooting && s_fire){
        
        if (shootingSound && shootingSound->soundId < 0) {
            shootingSound->playAsSound(true);
        }

        
        float phase = 25;
        s_fire->setOpacity( (sinf(counter*phase) + 1)/2.0 * 255);
        
        s_fire->setScaleX( (cosf(counter*phase) + 1)/2.0 * 0.8 + 1);
        s_fire->setScaleY( (sinf(counter*phase) + 1)/2.0 * 0.8 + 1);

        s_fire->setPosition( ccp( loc.x + shootingPoint.x + sinf(counter*phase)*5, loc.y + shootingPoint.y) );

        
        if (s_fire->getOpacity() > 200 && bullet_out == false){
            bullet_out = true;
            
            Sprite *s = s_bullet[ kPlayerMaxBullets-1 ];
            b2Body* b = p_bullet[ kPlayerMaxBullets-1 ];
            
            for (int i = kPlayerMaxBullets-1; i > 0; i--){
                bullet_velocity[ i ] = bullet_velocity[ i-1 ];
                s_bullet[ i ] = s_bullet[ i-1 ];
                p_bullet[ i ] = p_bullet[ i-1 ];
            }
            
            s_bullet[ 0 ] = s;
            p_bullet[ 0 ] = b;
            
            bullet_velocity[ 0 ] = ccp( 32.0*shootingPoint.x*bullet_speed , 32.0*shootingPoint.y*bullet_speed);
            if (s_bullet[ 0 ]) {
                s_bullet[ 0 ]->setPosition( ccp( loc.x + shootingPoint.x, loc.y +shootingPoint.y) );
                s_bullet[ 0 ]->setRotation( shooting_angle );
            }
            
        }
        
        if (s_fire->getOpacity() < 40) {
            bullet_out = false;
        }
    }
    else{
        if (shootingSound && shootingSound->soundId >= 0) {
            shootingSound->stopSound();
        }
        
        if (s_fire) {
            s_fire->setOpacity(0);
        }
    }
   


    
    Size size = Director::getInstance()->getWinSize();
    
    for (int i=0; i < kPlayerMaxBullets; i++) {
        if (p_bullet[ i ]) {
            Vec2 pp = s_bullet[ i ]->getPosition();
            p_bullet[ i ]->SetTransform(b2Vec2( pp.x*kPhysicsWorldScale(), pp.y*kPhysicsWorldScale()), 0);
        }
        
        if (ccpLength( bullet_velocity[ i ] ) > 0.1 && s_bullet[ i ]) {
            s_bullet[ i ]->setOpacity( 255 );
            Vec2 p = s_bullet[ i ]->getPosition();
            s_bullet[ i ]->setPosition( ccp( p.x+bullet_velocity[ i ].x*dd, p.y+bullet_velocity[ i ].y*dd) );
            
            if (s_bullet[ i ]->getPosition().x > size.width ||
                s_bullet[ i ]->getPosition().y > size.height ||
                s_bullet[ i ]->getPosition().x < 0 ||
                s_bullet[ i ]->getPosition().y < 0) {
                
                bullet_velocity[ i ] = ccp(0,0);
                s_bullet[ i ]->setOpacity( 0 );
                s_bullet[ i ]->setPosition( ccp(-400, -400) );
            }
        }
    }
    
    
}

void PZGCharacter::removeBullet(b2Body *body){
    for (int i=0; i < kPlayerMaxBullets; i++) {
        if(p_bullet[ i ] == body){
            bullet_velocity[ i ] = ccp(0,0);
            s_bullet[ i ]->setOpacity( 0 );
            s_bullet[ i ]->setPosition( ccp(-100, -100) );

            
            break;
        }
    }
}

