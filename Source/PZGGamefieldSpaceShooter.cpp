//
//  PZGGamefieldSpaceShooter.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#include "PZGGamefieldSpaceShooter.h"
#include "SharedData/PZGSharedData.h"

Scene* PZGGamefieldSpaceShooter::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldSpaceShooter *layer = PZGGamefieldSpaceShooter::create();
	scene->addChild(layer);
	return scene;
    
}

bool PZGGamefieldSpaceShooter::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    initPhysics( ccp(0, 0) );
    
    PZSettingsController *sc = PZSettingsController::shared();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();

    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplaySpaceShooter* gameInfoObject = (PZGGameplaySpaceShooter*)array->objectAtIndex(0);

    array = (__Array*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral *generalInfo = (PZGGameInfoGeneral*)array->objectAtIndex( 0 );
    
    character = PZGCharacter::create(sc->characterId, world, false );
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        
        if (generalInfo->isPortraitMode) {
            character->setShootingAngle( -90 );
            character->setLoc( ccp(size.width/2.0, size.height/5.0) );
        }
        else{
            character->setLoc( ccp(size.width/5.0, size.height/2.0) );
        }

        player_start = character->loc;
    }

    PZGGameFieldScene::init();
    
    restrictCoins = true;
    
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    

    speedScaleX = gameInfoObject->speedScaleX;
    speedScaleY = gameInfoObject->speedScaleY;
    if (character) {
        character->fire_speed = gameInfoObject->shootSpeed;
        character->bullet_speed = gameInfoObject->bulletSpeed;
    }
    
    autoshooting = gameInfoObject->autoshooting;
    
    this->setTouchEnabled(true);
    
    return true;
}

void PZGGamefieldSpaceShooter::reset(){
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

void PZGGamefieldSpaceShooter::update(float dt){
    PZGGameFieldScene::update( dt );
    
    if (gameOver || gamePause) {
        return;
    }
    
    float smooth = 0.2f;
    if (character) {
        Vec2 p = ccpLerp(ccpSub(character->loc, player_start),
                            ccp(player_target.x * speedScaleX, player_target.y * speedScaleY), smooth);
        character->setLoc( ccpAdd(p, player_start) );
    }
   
    if (autoshooting && !gamePause && !gameOver) {
        character->shooting = true;
    }
}




void PZGGamefieldSpaceShooter::ccTouchesBegan(__Set *pTouch, Event *pEvent){
 
    __SetIterator it = pTouch->begin();
    Touch* touch;
    
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);
        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        if (location.x > size.width*0.6 && autoshooting == false){
            if (character) {
                character->shooting = true;
            }

            if (!fire_touch) {
                fire_touch = touch;
            }
        }
        else{
            player_target_offset = ccpSub( Director::getInstance()->convertToGL(location),
                                          player_target);
            if (!move_touch) {
                move_touch = touch;
            }
        }
        it++;
    }
    
}

void PZGGamefieldSpaceShooter::ccTouchesMoved(__Set *pTouch, Event *pEvent){
    __SetIterator it = pTouch->begin();
    Touch* touch;
    
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);
        
        Vec2 location = touch->getLocationInView();
        Vec2 location_prev = touch->getPreviousLocationInView();
        location = Director::getInstance()->convertToGL(location);
        location_prev = Director::getInstance()->convertToGL(location_prev);
        
        Size size = Director::getInstance()->getWinSize();
        if (touch == fire_touch){
            if (location.x <= size.width*0.6) {
                if (character) {
                    character->shooting = false;
                }
            }
            else{
                if (character) {
                    character->shooting = true;
                }
            }
        }
        else
            if(touch == move_touch){
                player_target = ccpSub(location, player_target_offset);
                
                Vec2 p = ccpAdd(player_target, player_start) ;
                
                Size characterSize = character->s_body->getContentSize();
                
                if (p.x > size.width - (boundRight * size.width)) {
                    p.x = size.width - ((boundRight * size.width) + characterSize.width/2);
                }
                if (p.x < size.width * boundLeft) {
                    p.x = (size.width * boundLeft)+characterSize.width/2;
                }
                if (p.y > size.height - (boundDown * size.height)) {
                    p.y = size.height - ((boundDown * size.height)+characterSize.height/2);
                }
                if (p.y < size.height * boundUp) {
                    p.y = (size.height * boundUp)+characterSize.height/2;
                }
                
                player_target = ccpSub(p, player_start);
            }
        it++;
    }
}

void PZGGamefieldSpaceShooter::ccTouchesEnded(__Set *pTouch, Event *pEvent){

    __SetIterator it = pTouch->begin();
    Touch* touch;
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);
        
        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        if (touch == fire_touch){
            if (character) {
                character->shooting = false;
            }
            fire_touch = NULL;
        }
        else
            if( touch == move_touch){
                move_touch = NULL;
            }
        it++;
    }
}