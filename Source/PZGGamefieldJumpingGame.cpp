//
//  PZGGamefieldJumpingGame.cpp
//  ProjectMayhem
//
//  Created by Amit Srivastava on 29/04/13.
//
//

#include "PZGGamefieldJumpingGame.h"
#include "SharedData/PZGGameplayJumpingGame.h"
#include "constants.h"

USING_NS_AX;

Scene* PZGGamefieldJumpingGame::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldJumpingGame *layer = PZGGamefieldJumpingGame::create();
	scene->addChild(layer);
	return scene;
}

void PZGGamefieldJumpingGame::reset(){
    
    scrollDiff=0.0f;
    
    isInit = true;
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayJumpingGame* gameInfoObject = (PZGGameplayJumpingGame*)array->objectAtIndex(0);
    
    gf_speed_max = gameInfoObject->gameSpeedMax;
    gf_speed_increaser =  gameInfoObject->gameSpeedIncreaser;
    gf_speed = gameInfoObject->gameSpeedMin;
    
    gf_distance = 0.0;
    
    PZSettingsController* sc = PZSettingsController::shared();
    gf_coins = sc->coins;
    
    if (platform) {
        platform->destoryPhysics( );
        platform->removeFromParentAndCleanup( true );
    }
    
    Scene* p = PZGGamefieldJumpingPlatforms::scene();
    this->addChild(p, character_depth_z-1);
    platform = (PZGGamefieldJumpingPlatforms*)p->getChildren().at( 0 );
    platform->world = world;
    platform->platfromsBetweenDistance = gameInfoObject->platfromsBetweenDistance;
    
    if (character) {
        character->s_body->setFlipX(false);
        character->reset();
        Size size = Director::getInstance()->getWinSize();
        //character->setLoc( ccp(size.width/5.0, size.height+60) );
        character->jump_permanently = true;
    }
    
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        if (gameStartSound) {
            gameStartSound->stopSound();
        }
        gameStartSound = (PZGSoundData*)sounds->objectAtIndex( kSoundIDGameStart );
        gameStartSound->playAsSound( false );
    }
}


bool PZGGamefieldJumpingGame::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayJumpingGame* gameInfoObject = (PZGGameplayJumpingGame*)array->objectAtIndex(0);
    tiltScale = gameInfoObject->tiltScale;
    useAccelerometer = gameInfoObject->useAccelerometer;
    
    initPhysics( ccp(0, -50) );
    
    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, true );
    if (character) {
        character->setLoc( ccp(80.0, 300.0) );
        character->jump_height = gameInfoObject->characterJumpHeight * 15;
    }
    
    PZGGameFieldScene::init();
    
    Scene* p = PZGGamefieldJumpingPlatforms::scene();
    this->addChild(p, character_depth_z-1);
    platform = (PZGGamefieldJumpingPlatforms*)p->getChildren().at( 0 );
    platform->world = world;
    
    
    this->setTouchEnabled(true);
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    
    if (useAccelerometer) {
        this->setAccelerometerEnabled(true);
    }
    
#else
    
    useAccelerometer = false;
    
#endif

    isInit = true;
    
    return true;
}



void PZGGamefieldJumpingGame::update(float dt){
    
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
        //gf_distance += gf_speed * dt;
        
        platform->speed = gf_speed;
        platform->distance = gf_distance;
        platform->update( dt );
        
//        Size size = Director::getInstance()->getWinSize();
        
        if (isInit) {
            isInit = false;
            
            Sprite* firstPlatform = platform->getPlatformForIndex(-1);
            
            if (firstPlatform) {
                Size size =firstPlatform->getContentSize();
                Vec2 position = firstPlatform->getPosition();
                
                
                position = ccp(position.x+((size.width-character->getContentSize().width)/2),position.y+size.height+50);
                
                character->setLoc(position);
            }
            
        }
        
        //will comment this out for now
        if (!player_target_offset.equals(Vec2Zero)) {
            
            ax::Vec2   player_old_position = character->loc;
            
            ax::Vec2   player_target = character->loc;
            
            player_target.x+=player_target_offset.x;
            player_target.y+=player_target_offset.y;
            
            if (player_target.x < player_old_position.x) {
                character->s_body->setFlipX(true);
            }else{
                character->s_body->setFlipX(false);
            }
            
            player_target_offset.setPoint(0, 0);
        }
        
        char c[16];
        sprintf( c, "%d", (int)gf_distance);
        distance_label->setString( c );
        character->update( dt );
        
        
        Size size = Director::getInstance()->getWinSize();
        
        //printf("%f,%f,%f\n",character->loc.x,character->loc.y,size.height/2);
        
        if (character->loc.x > size.width) {
            character->setLoc(ccp(character->loc.x-size.width,character->loc.y));
        }else if(character->loc.x < 0){
            character->setLoc(ccp(size.width-character->loc.x,character->loc.y));
        }
        
        //lets start fro the beggining
        if( character->loc.y > (size.height/2)){
            scrollDiff = character->loc.y-(size.height/2); //this is right and i'm agree with this.
            platform->scroll(scrollDiff,dt);
            character->setLoc( ccp( character->loc.x, character->loc.y - scrollDiff) ) ;
            gf_distance += scrollDiff*0.1;
            
            //printf("%f - %f\n",gf_speed,scrollDiff);
            
            for (int i=0; i < parallax_n; i++) {
                if (parallax[ i ]) {
                    parallax[ i ]->updateWithVelocity(ccp(-gf_speed,-scrollDiff), dt*kDeviceScale()*2*(1/parallax[i]->getScale()));
                }
            }
        }
        
        // one sec ...

        
        if (character->loc.y < -20.0) {
            gameOverCallBack( true );
        }
        if (character->loc.x < -20.0) {
            gameOverCallBack( true );
        }
        
    }
    
    //so here is place where we need to set our vector to zero. but we will do that in smooth way.
//    so you tap and got a vector  then you character will flight until gravity will change this vector or collision.
    //this is why we need to descrese vector. so charactter will jump stright up like in doodle jump
    
//    player_target_offset = ccp( player_target_offset.x*0.9, player_target_offset.y );
//    
//    //as first step we need to know current velocity. becouse we have jumping and we need jumping velocity
//    b2Vec2 vel = character->p_body->GetLinearVelocity();
//    
//    //apply force and set linear velocity is pretty the same methods. but in forces you can apply to different point. it is not necessary in our case. and better to work with velocity values. as we get VElocity at previous line
//    character->p_body->SetLinearVelocity( b2Vec2( 0.4f * player_target_offset.x, vel.y) );
//    
//    //we need to aply and keep velocty every frame instaed of assign only once 
    
    if (coins_add_counter > 0) {
        coins_add_counter -= dt;
        coins_add_velocity -= dt*3;
        
        Vec2 p = coins_add_label->getPosition();
        coins_add_label->setPosition( ccp( p.x + dt*10 , p.y + dt*coins_add_velocity*100 ) );
        coins_add_label->setOpacity( clampf(coins_add_counter * 255, 0, 255)  );
    }
}

void PZGGamefieldJumpingGame::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    if (character && !useAccelerometer) {

        ax::Vec2   player_target = character->loc;
        
        Touch* touch = (Touch*)( pTouch->anyObject() );
        
        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        Vec2 newLocation = Director::getInstance()->convertToGL(location);
        
        player_target_offset = ccpSub(newLocation ,
                                      player_target);
        
       //as first step we need to know current velocity. because we have jumping and we need jumping velocity
        b2Vec2 vel = character->p_body->GetLinearVelocity();
        
        float factor = 0.6;
        
        if (newLocation.x > player_target_offset.x) {
            character->p_body->ApplyForce(b2Vec2((player_target_offset.x*factor*tiltScale),vel.y), character->p_body->GetPosition());
        }else{
            character->p_body->ApplyForce(b2Vec2(-(player_target_offset.x*factor*tiltScale),vel.y), character->p_body->GetPosition());
        }
    }
}

void PZGGamefieldJumpingGame::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    if (character) {
//        character->jumpEnd();
    }
}

void PZGGamefieldJumpingGame::didAccelerate(Acceleration* pAccelerationValue) {
    if (character) {
        Size size = Director::getInstance()->getWinSize();
        
        ax::Vec2   player_target = character->loc;
        
        
        double acc_x = pAccelerationValue->x;
        
        if (acc_x < -0.1) {
            acc_x = -0.1;
        }
        
        if (acc_x > 0.1) {
            acc_x = 0.1;
        }
        
        float factor = 0.6;
        
        Vec2 newLocation = ccp(player_target.x+(acc_x * (size.width/4)*factor*tiltScale),player_target.y);
        
        player_target_offset = ccpSub(newLocation ,
                                      player_target);
        
        //as first step we need to know current velocity. because we have jumping and we need jumping velocity
        b2Vec2 vel = character->p_body->GetLinearVelocity();
        
        character->p_body->ApplyForce(b2Vec2((player_target_offset.x),vel.y), character->p_body->GetPosition());
        
//        if (newLocation.x > player_target_offset.x) {
//            character->p_body->ApplyForce(b2Vec2((player_target_offset.x),vel.y), character->p_body->GetPosition());
//        }else{
//            character->p_body->ApplyForce(b2Vec2(-(player_target_offset.x),vel.y), character->p_body->GetPosition());
//        }
        
        //printf("%f,%f\n",player_target_offset.x,player_target_offset.y);
        
    }
}
