//
//  ContactListener.cpp
//  PZPlayer
//
//  Created by Nik Rudenko on 6/24/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#include "PZLegacyCompat.h"
#include "PZGContactListener.h"

#include "PZSettingsController.h"

#include "PZGGameFieldScene.h"
#include "PZGCharacter.h"
#include "PZGCoin.h"
#include "PZGObstacle.h"
#include "PZGPlatfrom.h"
#include "PZGGamefieldPlatforms.h"

PZGContactListener::PZGContactListener(){
}

PZGContactListener::~PZGContactListener(){
    
}
void PZGContactListener::EndContact( b2Contact* contact){
    b2Fixture* fA = contact->GetFixtureA();
	b2Fixture* fB = contact->GetFixtureB();
    
    //////////////////////////
    // Platforms
    
    if (((Node*)fA->GetUserData())->getTag() == kPlatfromTag) {
        if (((Node*)fB->GetUserData())->getTag() == kCharacterTag) {
            if (fB->GetBody()->IsBullet() != true) {
                PZGCharacter *character = ((PZGCharacter*)fB->GetUserData());
                character->jump_nongrounded = true;
            }
        }
    }
    
    if (((Node*)fB->GetUserData())->getTag() == kPlatfromTag) {
        if (((Node*)fA->GetUserData())->getTag() == kCharacterTag) {
            if (fA->GetBody()->IsBullet() != true) {
                PZGCharacter *character = ((PZGCharacter*)fA->GetUserData());
                character->jump_nongrounded = true;
            }
        }
    }
}

void PZGContactListener::BeginContact( b2Contact* contact){
    
    b2Fixture* fA = contact->GetFixtureA();
	b2Fixture* fB = contact->GetFixtureB();
    
    //////////////////////////
    // Platforms
    
    if (((Node*)fA->GetUserData())->getTag() == kPlatfromTag) {
        if (((Node*)fB->GetUserData())->getTag() == kCharacterTag) {
            
            PZGCharacter *character = ((PZGCharacter*)fB->GetUserData());
            
            b2Vec2 vel = character->p_body->GetLinearVelocity();
            
            if (vel.y < 0) {
                character->jump_grounded();
            }
            
        }
    }
    
    if (((Node*)fB->GetUserData())->getTag() == kPlatfromTag) {
        if (((Node*)fA->GetUserData())->getTag() == kCharacterTag) {
            
            PZGCharacter *character = ((PZGCharacter*)fA->GetUserData());
            
            b2Vec2 vel = character->p_body->GetLinearVelocity();
            
            if (vel.y < 0) {
                character->jump_grounded();
            }
            
        }
    }
    
    //////////////////////////
    // Kid Mode Check
    PZSettingsController *sc = PZSettingsController::shared();
    if (sc->kidMode) {
        contact->SetEnabled( false );
        return;
    }
    
    
    
    ///////////////////////
    // Obstacles
    
    if (((Node*)fA->GetUserData())->getTag() == kObstacleTag) {
        PZGObstacle *e =(PZGObstacle*)fA->GetUserData();
        if (fB->GetBody()->IsBullet()) {
            if (!e->dead){
                ((PZGCharacter*)fB->GetUserData())->removeBullet( fB->GetBody() );
            }
            e->damage( 1 );            
        }
        else{
            if (e->dead == false && e->destroyCharacter == true) {
                PZGGameFieldScene::shared()->gameOverCallBack( true );
            }
        }
    }
    
    if (((Node*)fB->GetUserData())->getTag() == kObstacleTag) {
        PZGObstacle *e =(PZGObstacle*)fB->GetUserData();
        if (fA->GetBody()->IsBullet()) {
            if (!e->dead){
                ((PZGCharacter*)fA->GetUserData())->removeBullet( fA->GetBody() );
            }
            e->damage( 1 );
        }
        else{
            if (e->dead == false && e->destroyCharacter == true) {
                PZGGameFieldScene::shared()->gameOverCallBack( true );
            }
        }
    }
    

    ///////////////////////
    // Coins
    
    if (((Node*)fA->GetUserData())->getTag() == kCoinTag) {
        if (fB->GetBody()->IsBullet() == false) {
            PZGGameFieldScene::shared()->coinPickup( (PZGCoin*)fA->GetUserData() );
            
            ((PZGCharacter*)fB->GetUserData())->coinPicked = true;
        }
    }
    
    if (((Node*)fB->GetUserData())->getTag() == kCoinTag) {
        if (fA->GetBody()->IsBullet() == false) {
            PZGGameFieldScene::shared()->coinPickup( (PZGCoin*)fB->GetUserData() );
            
            ((PZGCharacter*)fA->GetUserData())->coinPicked = true;
        }
    }

    
    
    contact->SetEnabled( false );
}

void PZGContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold){
    b2Fixture* fA = contact->GetFixtureA();
	b2Fixture* fB = contact->GetFixtureB();
    
    if (((Node*)fA->GetUserData())->getTag() == kObstacleTag) {
        if (((PZGObstacle*)fA->GetUserData())->dead == true) {
            contact->SetEnabled( false );
        }
    }
    if (((Node*)fB->GetUserData())->getTag() == kObstacleTag) {
        if (((PZGObstacle*)fB->GetUserData())->dead == true) {
            contact->SetEnabled( false );
        }
    }
    
    if (((Node*)fA->GetUserData())->getTag() == kPlatfromTag) {
        if (((Node*)fB->GetUserData())->getTag() == kCharacterTag) {
            if (fB->GetBody()->IsBullet() != true) {
                if (((PZGCharacter*)fB->GetUserData())->getPositionY() < ((PZGPlatfrom *)fA->GetUserData())->sprite->getPositionY()) {
                    contact->SetEnabled( false );
                }
            }
        }
    }
    
    if (((Node*)fB->GetUserData())->getTag() == kPlatfromTag) {
        if (((Node*)fA->GetUserData())->getTag() == kCharacterTag) {
            if (fA->GetBody()->IsBullet() != true) {
                if (((PZGCharacter*)fA->GetUserData())->getPositionY() < ((PZGPlatfrom *)fB->GetUserData())->sprite->getPositionY()) {
                    contact->SetEnabled( false );
                }
            }
        }
    }
}
