//
//  PZGGamefieldBasicAvoidance.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/18/12.
//
//

#include "PZGGamefieldBasicAvoidance.h"
#include "SharedData/PZGSharedData.h"

USING_NS_AX;

Scene* PZGGamefieldBasicAvoidance::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldBasicAvoidance *layer = PZGGamefieldBasicAvoidance::create();
	scene->addChild(layer);
	return scene;
}

bool PZGGamefieldBasicAvoidance::init()
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
    PZGGameplayBasicAvoidance* gameInfoObject = (PZGGameplayBasicAvoidance*)array->objectAtIndex(0);
    
    speedScaleX = gameInfoObject->speedScaleX;
    speedScaleY = gameInfoObject->speedScaleY;
    
    this->setTouchEnabled(true);
    
    return true;
}

void PZGGamefieldBasicAvoidance::reset(){
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
    
    player_target = ccp(0, 0);
}

void PZGGamefieldBasicAvoidance::update(float dt){
    PZGGameFieldScene::update( dt );
    
    if (gameOver || gamePause) {
        return;
    }
    
    float smooth = 0.2f;
    Vec2 p = ccpLerp(ccpSub(character->loc, player_start),
                        ccp(player_target.x * speedScaleX, player_target.y * speedScaleY), smooth);
    character->setLoc( ccpAdd(p, player_start) );


}


void PZGGamefieldBasicAvoidance::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    Touch* touch = (Touch*)( pTouch->anyObject() );
    Vec2 location = touch->getLocationInView();
    Size size = Director::getInstance()->getWinSize();
    
    player_target_offset = ccpSub( Director::getInstance()->convertToGL(location),
                                  player_target);
}

void PZGGamefieldBasicAvoidance::ccTouchesMoved(__Set *pTouch, Event *pEvent){
    
    __SetIterator it = pTouch->begin();
    Touch* touch;
    
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);
        
        Vec2 location = touch->getLocationInView();
        Vec2 location_prev = touch->getPreviousLocationInView();
        location = Director::getInstance()->convertToGL(location);
        location_prev = Director::getInstance()->convertToGL(location_prev);
        
        Size size = Director::getInstance()->getWinSize();
        player_target = ccpSub(location, player_target_offset);
        
        Vec2 p = ccpAdd(player_target, player_start) ;
        
        if (p.x > size.width - boundRight * size.width) {
            p.x = size.width - boundRight * size.width;
        }
        if (p.x < size.width * boundLeft) {
            p.x = size.width * boundLeft;
        }
        if (p.y > size.height - boundDown * size.height) {
            p.y = size.height - boundDown * size.height;
        }
        if (p.y < size.height * boundUp) {
            p.y = size.height * boundUp;
        }
        player_target = ccpSub(p, player_start);
    }
    
}