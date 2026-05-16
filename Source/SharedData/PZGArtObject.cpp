//
//  PZArtObject_cpp.cpp
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "PZGArtObject.h"
#include "Contants_data.h"
#include "constants.h"
#include "PZLegacyCompat.h"

USING_NS_AX;

PZGArtObject::PZGArtObject(){
    no_of_sprites = 0;
    animationSpeed = 1.0;
}

PZGArtObject::~PZGArtObject(){
   
    
}

__Array* PZGArtObject::getResourceFrameNames(){
    
    __Array* array = __Array::create();

    if (!key) {
        AXLOGW("PZGArtObject::getResourceFrameNames: key is null");
        return array;
    }
    
    for (int i=0; i < no_of_sprites; i++) {
        __String* s = __String::createWithFormat("%s_%d_%d", key->getCString(), index, i);
        array->addObject( s );
    }
    
    return array;
}

ax::Animate*  PZGArtObject::getResourceAnimate(){

    if (no_of_sprites > 0) {
        if (!key) {
            AXLOGW("PZGArtObject::getResourceAnimate: key is null (index={}, sprites={})", index, no_of_sprites);
            return nullptr;
        }
        Animation* anim = Animation::create();
        for (int i=0; i < no_of_sprites; i++) {
            __String* s = __String::createWithFormat("%s_%d_%d", key->getCString(), index, i);
            SpriteFrame* f = SpriteFrameCache::getInstance()->getSpriteFrameByName(s->m_sString.c_str());
            if (f) {
                anim->addSpriteFrame(f);
            } else {
                AXLOGW("PZGArtObject::getResourceAnimate: missing frame '{}'", s->m_sString);
            }
        }
        if (anim->getFrames().empty()) {
            AXLOGW("PZGArtObject::getResourceAnimate: no frames for key='{}' index={}", key ? key->getCString() : "(null)", index);
            return nullptr;
        }
        anim->setDelayPerUnit(1/ (15.0 * animationSpeed));
        Animate *a = Animate::create(anim);
        a->setTag(kDefaultAnimationTag);
        return a;
    }
    else{
        return NULL;
    }   
}

ax::Sprite *  PZGArtObject::getResource(){
    Sprite *sprite;

    if (!key) {
        AXLOGW("PZGArtObject::getResource: key is null");
        return nullptr;
    }

    if(no_of_sprites > 1) {

        __String *spriteName = __String::createWithFormat("%s_%d_%d", key->getCString(), index, 0);
        
        SpriteFrame* pFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteName->getCString());
        if (pFrame == NULL) {
            return NULL;
        }
        
        sprite = Sprite::createWithSpriteFrameName( spriteName->getCString() );
        
        Animation* anim = Animation::create();
        for (int i=0; i < no_of_sprites; i++) {
            __String* s = __String::createWithFormat("%s_%d_%d", key->getCString(), index, i);
            SpriteFrame* f = SpriteFrameCache::getInstance()->getSpriteFrameByName(s->m_sString.c_str());
            if (f) {
                anim->addSpriteFrame(f);
            } else {
                AXLOGW("PZGArtObject::getResource: missing frame '{}'", s->m_sString);
            }
        }
        if (!anim->getFrames().empty()) {
            anim->setDelayPerUnit(1/(15.0 * animationSpeed));
            Animate *a = Animate::create(anim);
            if (a) {
                CCAction *action = CCRepeatForever::create(a);
                if (action) {
                    action->setTag(10);
                    sprite->runAction(action);
                }
            }
        }
    }
    else{
        
        __String *spriteName = __String::createWithFormat("%s_%d_%d", key->getCString(), index, 0);
        
        SpriteFrame* pFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteName->getCString());
        if ( pFrame == NULL ) {
            return NULL;
        }
        else{
            sprite = Sprite::createWithSpriteFrameName( spriteName->getCString() );
//            printf("Sprite Name : %s \n", spriteName->getCString());
            if(!spriteName->compare("InterfaceMM_1_0") ){
//                printf("Match\n");
                Sprite *mySprite = Sprite::create("gameLogo.png");
                return mySprite;
            }
        }
    }
    
    
    return sprite;
}

PZGArtObject* PZGArtObject::createWithDictionary(ax::__Dictionary * dictionary)
{
    PZGArtObject *artObj = new PZGArtObject();
    
    artObj->name = (ax::__String*)dictionary-> valueForKey(ART_OBJECT_NAME);
    artObj->className = dictionary->valueForKey(OBJECT_CLASS_NAME);
    artObj->key = ( __String* )dictionary->valueForKey(ART_OBJECT_KEY);
    artObj->no_of_sprites = (dictionary->valueForKey(ART_NO_OF_SPRITES))->intValue();
    artObj->index = dictionary->valueForKey("index")->intValue();
    

    
    return artObj;
}
