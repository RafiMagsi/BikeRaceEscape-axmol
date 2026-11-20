//
//  PZGamefieldBasicRunner.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#include "PZLegacyCompat.h"
#include "PZRandom.h"
#include "V2/V2Replay.h"
#include "V2/V2Metrics.h"
#include "PZGGamefieldBasicRunner.h"
#include "PZGGamefieldPlatforms.h"
#include "SharedData/PZGGameplayBasicRunner.h"
#include "constants.h"
#include "Features/PZRunDirector.h"
#include "Features/PZGameConfig.h"

USING_NS_AX;

Scene* PZGGamefieldBasicRunner::scene(){
    
	Scene *scene = Scene::create();
	if (!scene) {
	    AXLOGE("PZGGamefieldBasicRunner::scene: failed to create Scene");
	    return nullptr;
	}

	PZGGamefieldBasicRunner *layer = PZGGamefieldBasicRunner::create();
	if (!layer) {
	    // `create()` returns nullptr when `init()` fails. Never add a null child.
	    AXLOGE("PZGGamefieldBasicRunner::scene: failed to create layer (init returned false)");
	    return nullptr;
	}

	scene->addChild(layer);
	return scene;
}

// Jump apex in scene units. Gravity and the jump impulse live in physics units, so the real height depends on
// kPhysicsWorldScale() (0.05 on iOS vs 0.1 on Android): obstacle fairness and power-up heights must use this number,
// not a constant.
void PZGGamefieldBasicRunner::applyJumpApex(){
    if (!platform || !character || !world) {
        return;
    }
    const float g = std::fabs(world->GetGravity().y);
    if (g < 0.001f) {
        return;
    }
    platform->riderApex = (character->jump_height * character->jump_height) / (2.0f * g) / kPhysicsWorldScale();
}

void PZGGamefieldBasicRunner::reset(){
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    if (!gsd) {
        AXLOGE("PZGGamefieldBasicRunner::reset: gsd is null");
        return;
    }
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    if (!array || array->count() == 0) {
        AXLOGE("PZGGamefieldBasicRunner::reset: GameplaySettings missing/empty");
        return;
    }
    PZGGameplayBasicRunner* gameInfoObject = (PZGGameplayBasicRunner*)array->objectAtIndex(0);
    if (!gameInfoObject) {
        AXLOGE("PZGGamefieldBasicRunner::reset: gameInfoObject is null");
        return;
    }
    
    gf_speed_max = gameInfoObject->gameSpeedMax;
    gf_speed_increaser =  gameInfoObject->gameSpeedIncreaser;
    gf_speed = gameInfoObject->gameSpeedMin;
    
    gf_distance = 0.0;
    
    PZSettingsController* sc = PZSettingsController::shared();
    gf_coins = sc->coins;
    
    if (character) {
        character->reset();
        Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/5.0, size.height+60) );
        character->jump_grounded();
    }

    if (platform) {
        platform->destoryPhysics( );
        platform->removeFromParentAndCleanup( true );
    }
    
    platform = PZGGamefieldPlatforms::create();
    if (platform) {
        this->addChild(platform, character_depth_z - 1);
        platform->world = world;
        platform->platfromsBetweenDistance = gameInfoObject->platfromsBetweenDistance;
        applyJumpApex();
    } else {
        AXLOGE("PZGGamefieldBasicRunner::reset: failed to create PZGGamefieldPlatforms");
    }
    
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds) {
        if (gameStartSound) {
            gameStartSound->stopSound();
        }
        auto* tmp = (sounds->count() > kSoundIDGameStart)
            ? dynamic_cast<PZGSoundData*>(sounds->objectAtIndex(kSoundIDGameStart))
            : nullptr;
        if (tmp) {
            gameStartSound = tmp;
            gameStartSound->playAsSound(false);
        } else {
            AXLOGW("PZGGamefieldBasicRunner::reset: missing gameStartSound at index={} (sounds count={})",
                   kSoundIDGameStart, sounds->count());
        }
    }
}


bool PZGGamefieldBasicRunner::init()
{
	if ( !Layer::init() )	{
		return false;
	}

    AXLOGI("PZGGamefieldBasicRunner::init begin");
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    if (!gsd) {
        AXLOGE("PZGGamefieldBasicRunner::init: gsd is null");
        return false;
    }
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    if (!array || array->count() == 0) {
        AXLOGE("PZGGamefieldBasicRunner::init: GameplaySettings missing/empty");
        return false;
    }
    PZGGameplayBasicRunner* gameInfoObject = (PZGGameplayBasicRunner*)array->objectAtIndex(0);
    if (!gameInfoObject) {
        AXLOGE("PZGGamefieldBasicRunner::init: gameInfoObject is null");
        return false;
    }
    
    initPhysics( ccp(0, -50) );

    PZSettingsController *sc = PZSettingsController::shared();
    character = PZGCharacter::create(sc->characterId, world, true );
    if (character) {
        character->setLoc( ccp(80.0, 300.0) );
        character->jump_height = gameInfoObject->characterJumpHeight * 15;
    } else {
        AXLOGE("PZGGamefieldBasicRunner::init: failed to create character");
    }
    
    if (!PZGGameFieldScene::init()) {
        AXLOGE("PZGGamefieldBasicRunner::init: PZGGameFieldScene::init failed");
        return false;
    }
    
    platform = PZGGamefieldPlatforms::create();
    if (platform) {
        this->addChild(platform, character_depth_z - 1);
        platform->world = world;
        applyJumpApex();
    } else {
        AXLOGE("PZGGamefieldBasicRunner::init: failed to create PZGGamefieldPlatforms");
        return false;
    }

    
    this->setTouchEnabled(true);

    // Meta features (stages, power-ups, revive, results): see Source/Features/PZGameConfig.h.
    run = PZRunDirector::create(this);
    if (run) {
        this->addChild(run, 0);
        run->startRun();
    }

    AXLOGI("PZGGamefieldBasicRunner::init end");
    
    return true;
}



void PZGGamefieldBasicRunner::update(float dt){
    V2::Metrics::frame(dt);

    if (!character) {
        AXLOGE("PZGGamefieldBasicRunner::update: character is null");
        gameOverCallBack(true);
        return;
    }

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
    
    // Brief hit-stop on big impacts: the simulation holds for a few frames while effects keep playing.
    const bool hitStop = run && run->frozen();
    if (PZRand::fixedStep()) {
        V2::Replay::frame(gameOver);
        // A frozen / paused / finished world must not bank time: the banked slices would burst out at the first live frame.
        if (hitStop || gamePause || gameOver) _stepAcc = 0.0f;
        // Before GO the world is frozen, but the road still has to be generated: one zero-time update does exactly that.
        if (hitStop && platform && !platform->primed() && world && !world->IsLocked()) {
            platform->speed = 0.0f;
            platform->update(0.0f);
        }
    }

    // Fixed-step mode (challenge seeds): simulate in exact 1/60 s slices so a run is reproducible at any frame rate.
    int slices = 1;
    const float realDt = dt;
    if (PZRand::fixedStep()) {
        _stepAcc += realDt;
        slices = std::min(6, static_cast<int>(_stepAcc / PZRand::kFixedDt));
        _stepAcc -= slices * PZRand::kFixedDt;
    }
    // The hit-stop is re-read for every slice: a hit-stop that starts inside this frame must stop the following slices of the
    // same frame too, or a frame-rate stall would run slices that a smooth frame rate runs after the freeze.
    for (int slice = 0; slice < slices && !gameOver && !gamePause && !hitStop && !(slice > 0 && run && run->frozen()); ++slice) {
        const float dt = PZRand::fixedStep() ? PZRand::kFixedDt : realDt;
        if (!platform) {
            AXLOGE("PZGGamefieldBasicRunner::update: platform is null");
            return;
        }
        if (PZRand::fixedStep()) V2::Replay::feedDueInputs([this](bool p) { applyInput(p); });   // replay: inputs due at this tick
        
        // Forgiving jump: a tap just before touchdown fires the moment the rider lands.
        if (_jumpBuffer > 0.0f) {
            _jumpBuffer -= dt;
            if (character->jump_counter < 1) {
                character->jump();
                if (!_touchHeld) {
                    character->jumpEnd();
                }
                _jumpBuffer = 0.0f;
            }
        }

        world->Step(dt, 3, 3);
        
        gf_speed += gf_speed_increaser * dt;
        if (gf_speed > gf_speed_max) {
            gf_speed = gf_speed_max;
        }
        // Effective speed includes the turbo boost (1.0 normally).
        const float effSpeed = gf_speed * speedMultiplier();
        gf_distance += effSpeed * dt;
                
        char c[16];
        sprintf( c, "%d", (int)gf_distance);
        distance_label->setString( c );
        character->update( dt );
        
        for (int i=0; i < parallax_n; i++) {
            if (parallax[ i ]) {
                parallax[ i ]->updateWithVelocity(ccp(-effSpeed,0), dt*kDeviceScale()*2*(1/parallax[i]->getScale()));
            }
        }
         Size size = Director::getInstance()->getWinSize();
        character->setLoc( ccp(size.width/5.0, character->loc.y) );
        
        platform->speed = effSpeed;
        platform->distance = gf_distance;
        platform->update( dt );

        if (run) {
            run->update( dt );
            }
        
        if (PZRand::fixedStep() && character->p_body) {
            const b2Vec2 p = character->p_body->GetPosition(), v = character->p_body->GetLinearVelocity();
            V2::Replay::Snapshot snap;
            snap.x = p.x; snap.y = p.y; snap.vx = v.x; snap.vy = v.y;
            snap.distance = static_cast<float>(gf_distance); snap.speed = gf_speed;
            V2::Replay::afterSlice(snap, PZRand::drawHash());
        }

        if (character->loc.y < -20.0) {
            gameOverCallBack( true );
        }
        if (character->loc.x < -20.0) {
            gameOverCallBack( true );
        }

    }
    
    
    if (coins_add_counter > 0) {
        coins_add_counter -= dt;
        coins_add_velocity -= dt*3;
        
        Vec2 p = coins_add_label->getPosition();
        coins_add_label->setPosition( ccp( p.x + dt*10 , p.y + dt*coins_add_velocity*100 ) );
        coins_add_label->setOpacity( clampf(coins_add_counter * 255, 0, 255)  );
    }
}

void PZGGamefieldBasicRunner::ccTouchesBegan(__Set *pTouch, Event *pEvent){
    if (!character || (run && run->inputLocked())) {
        return;
    }
    injectInput(true);
}
void PZGGamefieldBasicRunner::ccTouchesEnded(__Set *pTouch, Event *pEvent){
    injectInput(false);
}

void PZGGamefieldBasicRunner::injectInput(bool press) {
    if (!V2::Replay::onLiveInput(press)) return;     // a replay is playing: live input is ignored
    applyInput(press);
}

void PZGGamefieldBasicRunner::applyInput(bool press) {
    if (!character) return;
    if (press) {
        _touchHeld = true;
        const bool wasGrounded = character->jump_counter < 1;
        character->jump();
        if (!wasGrounded && PZCfg::kEnableJumpBuffer) {
            _jumpBuffer = PZCfg::kJumpBuffer;
        }
    } else {
        _touchHeld = false;
        character->jumpEnd();
    }
}
