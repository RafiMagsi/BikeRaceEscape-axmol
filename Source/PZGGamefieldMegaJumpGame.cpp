//
//  PZGGamefieldMegaJumpGame.cpp
//  ProjectMayhem
//
//  Created by Amit Srivasatava on 15/05/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGamefieldMegaJumpGame.h"
#include "SharedData/PZGSharedData.h"

USING_NS_AX;

Scene* PZGGamefieldMegaJumpGame::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldMegaJumpGame *layer = PZGGamefieldMegaJumpGame::create();
	scene->addChild(layer);
	return scene;
}

bool PZGGamefieldMegaJumpGame::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    initPhysics( ccp(0, -5.8) );
    
    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, false );
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/2.0, size.height/2.0) );
        character->jump_height = 15;
    }
    player_start = character->loc;
    PZGGameFieldScene::init();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayMegaJumpGame* gameInfoObject = (PZGGameplayMegaJumpGame*)array->objectAtIndex(0);
    
    speedScaleX = gameInfoObject->speedScaleX;
    speedScaleY = gameInfoObject->speedScaleY;
    
    tiltScale = gameInfoObject->tiltScale;
    useAccelerometer = gameInfoObject->useAccelerometer;
    
    this->setTouchEnabled(true);
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    
    if (useAccelerometer) {
        this->setAccelerometerEnabled(true);
    }
    
#else
    
    useAccelerometer = false;
    
#endif
    
    return true;
}

void PZGGamefieldMegaJumpGame::reset(){
    PZGGameFieldScene::reset();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral *generalInfo = (PZGGameInfoGeneral*)array->objectAtIndex( 0 );
    
    if (character) {
        
        character->reset();
        character->p_body->SetLinearVelocity(b2Vec2(0,0));
        character->jump_height = 15;
        character->jump_grounded();
        character->jump();
        
        Size size = Director::getInstance()->getWinSize();
        if (generalInfo->isPortraitMode) {
            character->setLoc( ccp(size.width/2.0, size.height/2.0) );
        }
        else{
            character->setLoc( ccp(size.width/2.0, size.height/2.0) );
        }
        
        player_start = character->loc;
    }
    
    player_target = ccp(0, 0);
}

void PZGGamefieldMegaJumpGame::update(float dt){
    PZGGameFieldScene::update( dt );
    
    if (gameOver || gamePause) {
        return;
    }
    
//    float smooth = 0.2f;
//    Vec2 p = ccpLerp(ccpSub(character->loc, player_start),
//                        ccp(player_target.x * speedScaleX, player_target.y * speedScaleY), smooth);
//    character->setLoc( ccpAdd(p, player_start) );
    
    character->update(dt);
    
    Size size = Director::getInstance()->getWinSize();
    
    if ((character->loc.x+character->getContentSize().width) > size.width) {
        character->setLoc(ccp(size.width-character->getContentSize().width,character->loc.y));
    }else if(character->loc.x < 0){
        character->setLoc(ccp(0,character->loc.y));
    }
    
    if (character->coinPicked) {
        character->coinPicked = false;
        
        if( character->loc.y < (size.height/4)*3 && character->loc.y > (size.height/2)){
            character->jump_height = 5;
            character->jump_grounded();
            character->jump();
        }else if( character->loc.y < (size.height/2) && character->loc.y > (size.height/4)){
            character->jump_height = 15;
            character->jump_grounded();
            character->jump();
        }else if( character->loc.y < (size.height/4)){
            character->jump_height = 25;
            character->jump_grounded();
            character->jump();
        }

    }
    
    if (character->loc.y < -20.0) {
        gameOverCallBack( true );
    }
    if (character->loc.x < -20.0) {
        gameOverCallBack( true );
    }
}


void PZGGamefieldMegaJumpGame::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    
    if (character) {

        ax::Vec2   player_loc = character->loc;
        
        Touch* touch = (Touch*)( pTouch->anyObject() );
        
        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        Vec2 newLocation = Director::getInstance()->convertToGL(location);
        
        player_target_offset = ccpSub(newLocation ,
                                      player_loc);
        
        //as first step we need to know current velocity. because we have jumping and we need jumping velocity
        b2Vec2 vel = character->p_body->GetLinearVelocity();
        
        float factor = 0.6;
        
        if (player_loc.x >= 0 && (player_loc.x+character->getContentSize().width) <= size.width) {
            character->p_body->ApplyForce(b2Vec2((player_target_offset.x*factor*tiltScale*speedScaleX),vel.y*speedScaleY), character->p_body->GetPosition());
        }
        
//        character->jump_grounded();
//        character->jump();
    }
}

void PZGGamefieldMegaJumpGame::didAccelerate(Acceleration* pAccelerationValue) {
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        
        ax::Vec2   player_loc = character->loc;
        
        
        double acc_x = pAccelerationValue->x;
        
        if (acc_x < -0.1) {
            acc_x = -0.1;
        }
        
        if (acc_x > 0.1) {
            acc_x = 0.1;
        }
        
        float factor = 0.6;
        
        Vec2 newLocation = ccp(player_loc.x+(acc_x * (size.width/4)*factor*tiltScale),player_loc.y);
        
        player_target_offset = ccpSub(newLocation ,
                                      player_loc);
        
        //as first step we need to know current velocity. because we have jumping and we need jumping velocity
        b2Vec2 vel = character->p_body->GetLinearVelocity();
        
        if (player_loc.x >= 0 && (player_loc.x+character->getContentSize().width) <= size.width) {
            character->p_body->SetLinearVelocity(b2Vec2(player_target_offset.x*speedScaleX,vel.y*speedScaleY));
        }
        
    }
}
