//
//  PZGGamefieldTiltAvoidance.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/3/13.
//
//

#include "PZGGamefieldTiltAvoidance.h"
#include "SharedData/PZGSharedData.h"

USING_NS_AX;

Scene* PZGGamefieldTiltAvoidance::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldTiltAvoidance *layer = PZGGamefieldTiltAvoidance::create();
	scene->addChild(layer);
	return scene;
}

bool PZGGamefieldTiltAvoidance::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    
    initPhysics( ccp(0, 0) );
    
    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, false );
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/2.0, size.height/5.0) );
    }
    player_start = character->loc;
    PZGGameFieldScene::init();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayTiltAvoidance* gameInfoObject = (PZGGameplayTiltAvoidance*)array->objectAtIndex(0);
    tiltScale = gameInfoObject->tiltScale;
    
    this->setAccelerometerEnabled(true);
    this->setTouchEnabled(true);
    
    return true;
}

void PZGGamefieldTiltAvoidance::update(float dt){
    PZGGameFieldScene::update( dt );
    
    if (gameOver || gamePause) {
        return;
    }
    
    float smooth = 0.05f;
    Vec2 p = ccpLerp(ccpSub(character->loc, player_start),
                        ccp(player_target.x, player_target.y), smooth);
    character->setLoc( ccpAdd(p, player_start) );
    
    
}

void PZGGamefieldTiltAvoidance::didAccelerate(Acceleration* pAccelerationValue) {
    Size size = Director::getInstance()->getWinSize();
    player_target.x = pAccelerationValue->x * size.width*0.6 * tiltScale;
}

void PZGGamefieldTiltAvoidance::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    
    Touch* touch = (Touch*)( pTouch->anyObject() );
    
    Vec2 location = touch->getLocationInView();
    Size size = Director::getInstance()->getWinSize();
    
    player_target_offset = ccpSub( Director::getInstance()->convertToGL(location),
                                  player_target);
    
//    character->shooting = true;
    
}

void PZGGamefieldTiltAvoidance::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    
//    character->shooting = false;
}
