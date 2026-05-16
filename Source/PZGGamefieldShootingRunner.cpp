//
//  PZGGamefieldShootingRunner.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/23/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGGamefieldShootingRunner.h"

Scene* PZGGamefieldShootingRunner::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldShootingRunner *layer = PZGGamefieldShootingRunner::create();
	scene->addChild(layer);
	return scene;
}

void PZGGamefieldShootingRunner::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    __SetIterator it = pTouch->begin();
    Touch* touch;
    for( int iTouchCount = 0; iTouchCount < pTouch->count(); iTouchCount++ ){
        touch = (Touch*)(*it);

        Vec2 location = touch->getLocationInView();
        Size size = Director::getInstance()->getWinSize();
        
        if ( location.x/size.width > 0.5) {
            if (character) {
                character->shooting = true;
            }
            
            if (!fire_touch) {
                fire_touch = touch;
            }
        }
        else{
            character->jump();
            
            if (!move_touch) {
                move_touch = touch;
            }
        }
        
        it++;
    }

}


void PZGGamefieldShootingRunner::ccTouchesEnded(__Set *pTouch, Event *pEvent){
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
                character->jumpEnd();
            }
        
        it++;
    }
}
