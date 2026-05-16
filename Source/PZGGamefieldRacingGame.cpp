//
//  PZGGamefieldRacingGame.cpp
//  ProjectMayhem
//
//  Created by Amit Srivastava on 20/05/13.
//
//

#include "PZGGamefieldRacingGame.h"
#include "SharedData/PZGSharedData.h"
#include "PZLegacyCompat.h"
#include "constants.h"

USING_NS_AX;

Scene* PZGGamefieldRacingGame::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldRacingGame *layer = PZGGamefieldRacingGame::create();
	scene->addChild(layer);
	return scene;
}

bool PZGGamefieldRacingGame::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    isTouched = false;
    
    initPhysics( ccp(0, 0) );
    
    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, false );
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/2.0, size.height/5.0) );
    }
    player_start = character->loc;
    
    PZGGameFieldScene::init();
    
//    restrictObstacles = true;)
    restrictCoins = true;
    preventObstaclesOverlappingObstackes = true;
    
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayRacingGame* gameInfoObject = (PZGGameplayRacingGame*)array->objectAtIndex(0);
    tiltScale = gameInfoObject->tiltScale;
    useAccelerometer = gameInfoObject->useAccelerometer;
    boostMultiplier =  gameInfoObject->boostMultiplier;
    
    s_body_boostUp = gameInfoObject->boostupArtObject->getResource();
    if (s_body_boostUp) {
        
        s_body_boostUp->setAnchorPoint( ccp(0.5,0.5) );
        s_body_boostUp->setVisible(isTouched);
        int z = character->getLocalZOrder();
        
        this->addChild(s_body_boostUp, z+1);
    }
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    
    if (useAccelerometer) {
        this->setAccelerometerEnabled(true);
    }
    
#else
    
    useAccelerometer = false;
    
#endif
    
    this->setTouchEnabled(true);
    
    //this->setDebugModeEnabled(true);
    
    return true;
}

void PZGGamefieldRacingGame::update(float dt){
    if (isTouched) {
        
        float dt1=dt * 2;
        
        if (boostMultiplier > 0) {
            dt1*=boostMultiplier;
        }
        
        PZGGameFieldScene::update( dt1);
    }else{
        PZGGameFieldScene::update( dt );
    }
    
    if (gameOver || gamePause) {
        
        isTouched = false;
        
        if (s_body_boostUp) {
            s_body_boostUp->setVisible(isTouched);
        }
        
        return;
    }
    
    float smooth = 0.05f;
    Vec2 p = ccpLerp(ccpSub(character->loc, player_start),
                        ccp(player_target.x, player_target.y), smooth);
    character->setLoc( ccpAdd(p, player_start) );
    
    character->update(dt);
    
    Size size = Director::getInstance()->getWinSize();
    
    Size characterSize = character->s_body->getContentSize();
    
//    if ((character->loc.x+characterSize.width) > size.width) {
//        character->setLoc(ccp(size.width-characterSize.width,character->loc.y));
//    }else if(character->loc.x-characterSize.width/2 < 0){
//        character->setLoc(ccp(0,character->loc.y));
//    }
//    
    
    if (s_body_boostUp) {
        s_body_boostUp->setVisible(isTouched);
        Vec2 loc = character->loc;
        loc.y-=characterSize.height/2.0;
        s_body_boostUp->setPosition(loc);
        
    }
}

void PZGGamefieldRacingGame::didAccelerate(Acceleration* pAccelerationValue) {
    Size size = Director::getInstance()->getWinSize();
    player_target.x = pAccelerationValue->x * size.width*0.6 * tiltScale;
    
    Vec2 p = ccpAdd(player_target, player_start);
    
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

void PZGGamefieldRacingGame::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    
//    Touch* touch = (Touch*)( pTouch->anyObject() );
//    
//    Vec2 location = touch->getLocationInView();
//    Size size = Director::getInstance()->getWinSize();
//    
//    player_target_offset = ccpSub( Director::getInstance()->convertToGL(location),
//                                  player_target);
    
    //    character->shooting = true;

    
    
    if (character) {
        
        isTouched = true;
        
        if (!useAccelerometer) {
            
            ax::Vec2   player_loc = character->loc;
            
            Touch* touch = (Touch*)( pTouch->anyObject() );
            
            Vec2 location = touch->getLocationInView();
            Size size = Director::getInstance()->getWinSize();
            
            Vec2 newLocation = Director::getInstance()->convertToGL(location);
            
            //        Vec2 p = ccpLerp(ccpSub(character->loc, player_start), newLocation, 1.0);
            
            player_target_offset = ccpSub(newLocation ,
                                          player_loc);
            
            //        //as first step we need to know current velocity. because we have jumping and we need jumping velocity
            //        b2Vec2 vel = character->p_body->GetLinearVelocity();
            //
            //        float factor = 0.6;
            //
            //        if (player_loc.x >= 0 && (player_loc.x+character->getContentSize().width) <= size.width) {
            //            character->p_body->ApplyForce(b2Vec2((player_target_offset.x*factor*tiltScale),vel.y), character->p_body->GetPosition());
            //        }
            
            float factor = 0.6;
            player_target.x =player_target_offset.x*factor*tiltScale;
            
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
    }
}

void PZGGamefieldRacingGame::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    
    isTouched = false;
    //    character->shooting = false;
}
