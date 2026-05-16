//
//  PZGGamefieldHelicopterGame.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#include "PZGGamefieldHelicopterGame.h"
#include "SharedData/PZGSharedData.h"

USING_NS_AX;

Scene* PZGGamefieldHelicopterGame::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldHelicopterGame *layer = PZGGamefieldHelicopterGame::create();
	scene->addChild(layer);
	return scene;
}

bool PZGGamefieldHelicopterGame::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    initPhysics( ccp(0, 0) );

    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, false );
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/5.0, size.height/2.0) );
    }
    player_start = character->loc;
    PZGGameFieldScene::init();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );

    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayHelicopterGame* gameInfoObject = (PZGGameplayHelicopterGame*)array->objectAtIndex(0);

    forceUpValue = gameInfoObject->forceUp;
    gravityValue = gameInfoObject->gravity;
    
    this->setTouchEnabled(true);
    
    return true;
}

void PZGGamefieldHelicopterGame::update(float dt){
    PZGGameFieldScene::update( dt );
    
    if (gameOver || gamePause) {
        return;
    }
    
    Size size = Director::getInstance()->getWinSize();
    
    if(characterForceUp){
        force += dt*80.0*forceUpValue;
    }
    else{
        force -= dt*80.0*gravityValue;
    }
    


    player_target = ccp(player_target.x, player_target.y+dt*force);
    
    Vec2 p = ccpAdd(player_target, player_start) ;
    if (p.x > size.width - boundRight * size.width) {
        p.x = size.width - boundRight * size.width;
    }
    if (p.x < size.width * boundLeft) {
        p.x = size.width * boundLeft;
    }
    if (p.y > size.height - boundDown * size.height) {
        p.y = size.height - boundDown * size.height;
        force = 0;
    }
    if (p.y < size.height * boundUp) {
        p.y = size.height * boundUp;
        force = 0;
    }
    
    player_target = ccpSub(p, player_start);
    
    float smooth = 0.2f;
    p = ccpLerp(ccpSub(character->loc, player_start),
                 ccp(player_target.x * character->speed.x, player_target.y * character->speed.y), smooth);
    character->setLoc( ccpAdd(p, player_start) );
    
}

void PZGGamefieldHelicopterGame::reset(){
    PZGGameFieldScene::reset();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral *generalInfo = (PZGGameInfoGeneral*)array->objectAtIndex( 0 );
    
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        if (generalInfo->isPortraitMode) {
            character->setLoc( ccp(size.width/2.0, size.height/5.0) );
        }
        else{
            character->setLoc( ccp(size.width/5.0, size.height/2.0) );
        }
        player_start = character->loc;
    }
    force = 0;
    player_target = ccp(0, 0);
}


void PZGGamefieldHelicopterGame::ccTouchesBegan(__Set *pTouch, Event *pEvent){

    characterForceUp = true;

}


void PZGGamefieldHelicopterGame::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    
    characterForceUp = false;
    
}