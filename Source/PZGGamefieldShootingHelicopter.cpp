//
//  PZGGamefieldShootingHelicopter.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#include "PZGGamefieldShootingHelicopter.h"

Scene* PZGGamefieldShootingHelicopter::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldShootingHelicopter *layer = PZGGamefieldShootingHelicopter::create();
	scene->addChild(layer);
	return scene;
}


bool PZGGamefieldShootingHelicopter::init(){
    PZGGamefieldHelicopterGame::init();

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    PZGGameplayShootingHelicopter* gameInfoObject = (PZGGameplayShootingHelicopter*)array->objectAtIndex(0);
    
    character->fire_speed = gameInfoObject->shootSpeed;
    character->bullet_speed = gameInfoObject->bulletSpeed;
    
    return true;
}

void PZGGamefieldShootingHelicopter::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    __SetIterator it = pTouch->begin();
    Touch* touch;
    
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);
        
        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        if ( location.x/size.width > 0.5) {
            character->shooting = true;
            if (!fire_touch) {
                fire_touch = touch;
            }
        }
        else{
            if (!move_touch) {
                move_touch = touch;
            }
            
            characterForceUp = true;
        }
        
        it++;
    }
    
}


void PZGGamefieldShootingHelicopter::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    
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
        else{
            if( touch == move_touch){
                move_touch = NULL;
               characterForceUp = false;
            }
        }
        it++;
    }
}