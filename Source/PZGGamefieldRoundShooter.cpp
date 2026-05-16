//
//  PZGamefieldRoundShooter.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/9/13.
//
//

#include "PZGGamefieldRoundShooter.h"

#include "SharedData/PZGSharedData.h"

Scene* PZGGamefieldRoundShooter::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldRoundShooter *layer = PZGGamefieldRoundShooter::create();
	scene->addChild(layer);
	return scene;
    
}

bool PZGGamefieldRoundShooter::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    initPhysics( ccp(0, 0) );
    
    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, false );
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/2.0, size.height/2.0) );
    }
    player_start = character->loc;
    fire_target = ccp(0, 1.0);
    
    PZGGameFieldScene::init();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayRoundShooter* gameInfoObject = (PZGGameplayRoundShooter*)array->objectAtIndex(0);

    character->fire_speed = gameInfoObject->shootSpeed;
    character->bullet_speed = gameInfoObject->bulletSpeed;
    
    
    this->setTouchEnabled(true);
    
    return true;
}

void PZGGamefieldRoundShooter::update(float dt){
    PZGGameFieldScene::update( dt );
    
    if (gameOver || gamePause) {
        return;
    }
    
    float smooth = 0.2f;
    Vec2 p = ccpLerp(ccpSub(character->loc, player_start),
                        ccp(player_target.x * 1, player_target.y * 1), smooth);
    character->setLoc( ccpAdd(p, player_start) );
    
    float angle = 0;
    angle = atan2f(-fire_target.y, fire_target.x);
    character->setAngle( CC_RADIANS_TO_DEGREES( angle ) );
    character->setShootingAngle( CC_RADIANS_TO_DEGREES( angle ) );
    
}




void PZGGamefieldRoundShooter::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    
    __SetIterator it = pTouch->begin();
    Touch* touch;
    
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);
        
        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        if (location.x > size.width*0.5){
            character->shooting = true;
            if (!fire_touch) {
                fire_touch = touch;
                fire_target_offset =  Director::getInstance()->convertToGL(location);
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

void PZGGamefieldRoundShooter::ccTouchesMoved(__Set *pTouch, Event *pEvent){
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
            fire_target = ccpSub(location, fire_target_offset);
        }
        else
            if(touch == move_touch){
                player_target = ccpSub(location, player_target_offset);
                
                Vec2 p = ccpAdd(player_target, player_start) ;
                
                if (p.x > size.width) {
                    p.x = size.width;
                }
                if (p.x < 0) {
                    p.x = 0;
                }
                if (p.y > size.height) {
                    p.y = size.height;
                }
                if (p.y < 0) {
                    p.y = 0;
                }
                player_target = ccpSub(p, player_start);
            }
        it++;
    }
}

void PZGGamefieldRoundShooter::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    __SetIterator it = pTouch->begin();
    Touch* touch;
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);
        
        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        if (touch == fire_touch){
            character->shooting = false;
            fire_touch = NULL;
        }
        else
            if( touch == move_touch){
                move_touch = NULL;
            }
        it++;
    }
}

void PZGGamefieldRoundShooter::addObstacle( int typeId ){
    if (obstacles_info == NULL || obstacles_info->count() == 0) {
        return;
    }
    
    if (obstacles[ kEnemyMax-1 ] != NULL) {
        obstacles[ kEnemyMax-1 ]->destroyPhysics( world );
        this->removeChild(obstacles[ kEnemyMax-1 ], true);
    }
    
    for (int i = kEnemyMax-1; i > 0; i--){
        obstacles[ i ] = obstacles[ i-1 ];
        obstacles_vector[ i ] = obstacles_vector[ i-1 ];
    }
    
	Size screen = Director::getInstance()->getWinSize();
    
    PZGObstacle *obstacle = PZGObstacle::create( (PZGArtObstacle*)obstacles_info->objectAtIndex( typeId ), world);
    if (obstacle == NULL) {
        return;
    }
    
    float angle = CCRANDOM_0_1() * M_PI *2.0;
    Vec2 p = ccp(sinf(angle), cosf(angle));
    obstacle->setPosition( ccp( screen.width*0.5 + p.x * screen.width*1.5,
                                screen.height*0.5 + p.y * screen.height*1.5) );
    
    Size s = Size(obstacle->sprite->getContentSize().width * obstacle->obstacle_scale, obstacle->sprite->getContentSize().height * obstacle->obstacle_scale);
    obstacle->setPosition( setOffScreenPosition( obstacle->getPosition(), s ) );
    
    Vec2 v = ccp( obstacle->getPosition().x - screen.width*0.5 + CCRANDOM_MINUS1_1()*50,
                     obstacle->getPosition().y - screen.height*0.5 + CCRANDOM_MINUS1_1()*50 );
    
    obstacle->velocityLinear = ccp(0,0);
    
    v = ccpNormalize( v );
    obstacles_vector[ 0 ] = v;
    
    this->addChild( obstacle, character_depth_z - (obstacles_info->count() - typeId) );
    
    obstacles[ 0 ] =  obstacle;
        
}

void PZGGamefieldRoundShooter::addCoin( int typeId ){
    if (coins_info == NULL || coins_info->count() == 0) {
        return;
    }
    
    if (coins[ kCoinMax-1 ] != NULL) {
        coins[ kCoinMax-1 ]->destroyPhysics( world );
        this->removeChild(coins[ kCoinMax-1 ], true);
    }
    
    for (int i = kCoinMax-1; i > 0; i--){
        coins[ i ] = coins[ i-1 ];
        coins_vector[ i ] = coins_vector[ i-1 ];
    }
    
    Size screen = Director::getInstance()->getWinSize();
    
    PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( typeId ), world);
    if (coin == NULL) {
        return;
    }
    
    float angle = CCRANDOM_0_1() * M_PI *2.0;
    Vec2 p = ccp(sinf(angle), cosf(angle));
    coin->setPosition( ccp( screen.width*0.5 + p.x * screen.width*1.5,
                            screen.height*0.5 + p.y * screen.height*1.5) );
    
    coin->setPosition( setOffScreenPosition( coin->getPosition(), coin->sprite->getContentSize() ) );
    
    Vec2 v = ccp( coin->getPosition().x - screen.width*0.5 + CCRANDOM_MINUS1_1()*50,
                     coin->getPosition().y - screen.height*0.5 + CCRANDOM_MINUS1_1()*50 );
    v = ccpNormalize( v );
    
    coins_vector[ 0 ] = v;
    
    
    this->addChild( coin, character_depth_z );
    
    coins[ 0 ] =  coin;
    
}

Vec2 PZGGamefieldRoundShooter::setOffScreenPosition( Vec2 pos, Size size){
    Size screen = Director::getInstance()->getWinSize();
    
    if (pos.x < 0) {
        pos.x = 0 - size.width;
    }
    if (pos.y < 0) {
        pos.y = 0 - size.height;
    }
    if (pos.x > screen.width) {
        pos.x = screen.width + size.width;
    }
    if (pos.y > screen.height) {
        pos.y = screen.height + size.height;
    }
    
    return pos;
}

void PZGGamefieldRoundShooter::updateObstacles( float dt ){
    float s =  dt*gf_speed*32;
    Size screen = Director::getInstance()->getWinSize();
    
    if (obstacles_info) {
        for (int i = 0; i < kEnemyMax; i++) {
            if (obstacles[ i ]) {
                
                Vec2 p = obstacles_vector[ i ];
                
                obstacles[ i ]->setPosition( ccp(obstacles[ i ]->getPosition().x - p.x*s*obstacles[ i ]->speed,
                                                 obstacles[ i ]->getPosition().y - p.y*s*obstacles[ i ]->speed) );
                obstacles[ i ]->update( dt );
                
                if (obstacles[ i ]->deleteReady) {
                    obstacles[ i ]->removeFromParentAndCleanup( true );
                    obstacles[ i ]->destroyPhysics( world );
                    obstacles[ i ] = NULL;
                }
            }
        }
    }
}


void PZGGamefieldRoundShooter::reset(){
    PZGGameFieldScene::reset();
    
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/2.0, size.height/2.0) );
    }
    player_start = character->loc;
    fire_target = ccp(0, 1.0);
    player_target = ccp(0, 0);
}

void PZGGamefieldRoundShooter::updateCoins( float dt ){
    float s =  dt*gf_speed*32;
    Size screen = Director::getInstance()->getWinSize();
    
    if (coins_info) {
        for (int i = 0; i < kCoinMax; i++) {
            if (coins[ i ]) {
                if (coins[ i ]->disabled == false) {
                                        
                    Vec2 p = coins_vector[ i ];
                    
                    coins[ i ]->setPosition( ccp(coins[ i ]->getPosition().x - p.x*s, coins[ i ]->getPosition().y - p.y*s) );
                    coins[ i ]->update( dt );
                    
                }
                else{
                    if (coins[ i ]->deleteReady) {
                        coins[ i ]->removeFromParentAndCleanup( true );
                        coins[ i ]->destroyPhysics( world );
                        coins[ i ] = NULL;
                    }
                }
            }
        }
    }
    
}