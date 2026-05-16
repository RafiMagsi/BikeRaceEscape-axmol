//
//  PZGamefieldBasicRunner.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGGamefieldBasicRunner.h"
#include "SharedData/PZGGameplayBasicRunner.h"
#include "constants.h"

USING_NS_AX;

Scene* PZGGamefieldBasicRunner::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldBasicRunner *layer = PZGGamefieldBasicRunner::create();
	scene->addChild(layer);
	return scene;
}

void PZGGamefieldBasicRunner::reset(){
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayBasicRunner* gameInfoObject = (PZGGameplayBasicRunner*)array->objectAtIndex(0);
    
    gf_speed_max = gameInfoObject->gameSpeedMax;
    gf_speed_increaser =  gameInfoObject->gameSpeedIncreaser;
    gf_speed = gameInfoObject->gameSpeedMin;
    
    gf_distance = 0.0;
    
    PZSettingsController* sc = PZSettingsController::shared();
    gf_coins = sc->coins;
    
    if (character) {
        character->reset();
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/5.0, size.height+60) );
        character->jump_grounded();
    }

    if (platform) {
        platform->destoryPhysics( );
        platform->removeFromParentAndCleanup( true );
    }
    
    Scene* p = PZGGamefieldPlatforms::scene();
    this->addChild(p, character_depth_z-1);
    platform = (PZGGamefieldPlatforms*)p->getChildren().at( 0 );
    platform->world = world;
    platform->platfromsBetweenDistance = gameInfoObject->platfromsBetweenDistance;
    
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        if (gameStartSound) {
            gameStartSound->stopSound();
        }
        gameStartSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDGameStart );
        gameStartSound->playAsSound( false );
    }
}


bool PZGGamefieldBasicRunner::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayBasicRunner* gameInfoObject = (PZGGameplayBasicRunner*)array->objectAtIndex(0);
    
    initPhysics( ccp(0, -50) );

    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, true );
    if (character) {
        character->setLoc( ccp(80.0, 300.0) );
        character->jump_height = gameInfoObject->characterJumpHeight * 15;
    }
    
    PZGGameFieldScene::init();
    
    Scene* p = PZGGamefieldPlatforms::scene();
    this->addChild(p, character_depth_z-1);
    platform = (PZGGamefieldPlatforms*)p->getChildren().at( 0 );
    platform->world = world;

    
    this->setTouchEnabled(true);
    
    return true;
}



void PZGGamefieldBasicRunner::update(float dt){

    if (tip_counter <= 3 ){
        tip_counter += dt;
        
        if (tip_counter > 3){
            if(tip1){
                tip1->removeFromParentAndCleanup( true );
                tip1 = NULL;
            }
            if (tip2) {
                tip2->removeFromParentAndCleanup( true );
                tip2 = NULL;
            }
        }
    }
    
    if (!gameOver && !gamePause) {
        
        world->Step(dt, 3, 3);
        
        gf_speed += gf_speed_increaser * dt;
        if (gf_speed > gf_speed_max) {
            gf_speed = gf_speed_max;
        }
        gf_distance += gf_speed * dt;
                
        char c[16];
        sprintf( c, "%d", (int)gf_distance);
        distance_label->setString( c );
        character->update( dt );
        
        for (int i=0; i < parallax_n; i++) {
            if (parallax[ i ]) {
                parallax[ i ]->updateWithVelocity(ccp(-gf_speed,0), dt*kDeviceScale()*2*(1/parallax[i]->getScale()));
            }
        }
         Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/5.0, character->loc.y) );
        
        platform->speed = gf_speed;
        platform->distance = gf_distance;
        platform->update( dt );
        
        if (character->loc.y < -20.0) {
            gameOverCallBack( true );
        }
        if (character->loc.x < -20.0) {
            gameOverCallBack( true );
        }

    }
    
    
    if (coins_add_counter > 0) {
        coins_add_counter -= dt;
        coins_add_velocity -= dt*3;
        
        Vec2 p = coins_add_label->getPosition();
        coins_add_label->setPosition( ccp( p.x + dt*10 , p.y + dt*coins_add_velocity*100 ) );
        coins_add_label->setOpacity( clampf(coins_add_counter * 255, 0, 255)  );
    }
}

void PZGGamefieldBasicRunner::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    if (character) {
        character->jump();
    }
}
void PZGGamefieldBasicRunner::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    if (character) {
        character->jumpEnd();
    }
}
