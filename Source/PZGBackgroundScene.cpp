//
//  BackgroundView.cpp
//  PZPlayer
//
//  Created by Nik Rudenko on 6/13/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//


#include "PZLegacyCompat.h"
#include "PZGBackgroundScene.h"
#include "constants.h"


USING_NS_AX;

Scene* PZGBackgroundScene::scene()
{
	Scene *scene = Scene::create();
	PZGBackgroundScene *layer = PZGBackgroundScene::create();
	scene->addChild(layer);
    
	return scene;
}

bool PZGBackgroundScene::init(){
	if ( !Layer::init() )	{
		return false;
	}
    Size screen = Director::getInstance()->getWinSize();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    backgroundLayers = (__Array*)gsd->artResource->objectForKey( "BackgroundLayers" );
       
    character = PZGCharacter::create( 0, NULL, false );
    
    parallax_n =  backgroundLayers->count();
    bool isCharacterAdded = false;
    for (int i=0; i < parallax_n; i++) {
        parallax[ i ] = CCParallaxScrollNode::create();
        PZGArtBackground *bgLayer = (PZGArtBackground*)backgroundLayers->objectAtIndex( i );
        if ( bgLayer->playGroundLayer == false ){
            
            Sprite *bgSprite1 = bgLayer->getResource();
            bgSprite1->setScale( bgLayer->scale );
            
            Sprite *bgSprite2 = bgLayer->getResource();
            bgSprite2->setScale( bgLayer->scale );
            
            parallax[ i ]->addInfiniteScrollXWithZ(0,
                                                   ccp(bgLayer->speed, 0.0),
                                                   ccp(0, (screen.height - bgSprite1->getContentSize().height)/2.0f + bgLayer->position*screen.height),
                                                   bgSprite1,
                                                   bgSprite2,
                                                   NULL);
            this->addChild( parallax[ i ], i);
        }
        else{
            if (!isCharacterAdded) {
                isCharacterAdded = true;
                this->addChild(character, i);
            }
        }
    }

    if (!isCharacterAdded) {
        this->addChild(character);
    }
    
    player_start = ccp( 60, screen.height/2 );
    
    
    this->schedule( AX_SCHEDULE_SELECTOR( PZGBackgroundScene::update ) );
    this->setTouchEnabled(true);
    enabled = true;
    
    return true;
}

void PZGBackgroundScene::update(float dt){
    
    if (enabled) {
        for (int i=0; i < parallax_n; i++) {
            parallax[ i ]->updateWithVelocity(ccp(-speed,0), dt);
        }
    }
    
    float smooth = 0.2f;
    Vec2 p = ccpLerp(ccpSub(character->loc, player_start),
                        ccp(player_target.x * character->speed.x, player_target.y * character->speed.y), smooth);
    character->loc = ccpAdd(p, player_start);
    character->update(dt);
}



void PZGBackgroundScene::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    Touch* touch = (Touch*)( pTouch->anyObject() );
    
    Vec2 location = touch->getLocationInView();
    Size size = Director::getInstance()->getWinSize();
    
    if (location.x > size.width*0.6){
        character->shooting = true;
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
}


void PZGBackgroundScene::ccTouchesMoved(__Set *pTouch, Event *pEvent){
    
    //    Touch* touch = (Touch*)( pTouch->anyObject() );
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
                character->shooting = false;
            }
            else{
                character->shooting = true;
            }
        }
        else
            if(touch == move_touch){
//                player_target_angle = (location_prev.y - location.y) * 3;
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
    }
    
}

void PZGBackgroundScene::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    //    Touch* touch = (Touch*)( pTouch->anyObject() );
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
    }
}
