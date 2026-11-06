//
//  PZGGamefieldPlatforms.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#include "PZLegacyCompat.h"
#include "PZGGamefieldPlatforms.h"
#include "constants.h"
#include "Features/PZGameConfig.h"
#include <algorithm>
#include <cmath>
#include <cstring>

USING_NS_AX;

PZGGamefieldPlatforms::PZGGamefieldPlatforms()
    : gameplayInfo(nullptr)
    , obstacles_info(nullptr)
    , coins_info(nullptr)
    , length(0.0f)
    , isSlowMoveMode(false)
    , speed(0.0f)
    , distance(0.0f)
    , world(nullptr)
    , platfromsBetweenDistance(0.0f)
    , prefillPending(true) {
    std::memset(platforms, 0, sizeof(platforms));
    std::memset(levels_info, 0, sizeof(levels_info));
    std::memset(obstacles, 0, sizeof(obstacles));
    std::memset(obstacles_spawn_counter, 0, sizeof(obstacles_spawn_counter));
    std::memset(coins, 0, sizeof(coins));
    std::memset(coins_spawn_counter, 0, sizeof(coins_spawn_counter));
}

PZGGamefieldPlatforms::~PZGGamefieldPlatforms() {
    for (int p = 0; p < kNumOFPlatforms; ++p) {
        AX_SAFE_RELEASE_NULL(levels_info[p]);
    }
}

Scene* PZGGamefieldPlatforms::scene(){
    AXLOGI("PZGGamefieldPlatforms::scene: Creating scene");

	Scene *scene = Scene::create();
	if (!scene) {
	    AXLOGE("PZGGamefieldPlatforms::scene: Failed to create Scene");
	    return nullptr;
	}

	PZGGamefieldPlatforms *layer = PZGGamefieldPlatforms::create();
	if (!layer) {
	    AXLOGE("PZGGamefieldPlatforms::scene: Failed to create PZGGamefieldPlatforms layer");
	    // Scene::create() returns an autoreleased object; never manually release it here.
	    return nullptr;
	}

    AXLOGI("PZGGamefieldPlatforms::scene: Adding layer to scene");
	scene->addChild(layer);
	AXLOGI("PZGGamefieldPlatforms::scene: Scene created successfully");
	return scene;
}

bool PZGGamefieldPlatforms::init()
{
	if ( !Layer::init() )	{
		return false;
	}

    AXLOGI("PZGGamefieldPlatforms::init: Starting");

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    if (!gsd) {
        AXLOGE("PZGGamefieldPlatforms::init: gsd is null");
        return false;
    }

    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey( "GameplaySettings" );
    if (!array || array->count() == 0) {
        AXLOGE("PZGGamefieldPlatforms::init: GameplaySettings not found or empty");
        return false;
    }

    gameplayInfo = (PZGGameplayBasicRunner*)array->objectAtIndex( 0 );
    if (!gameplayInfo) {
        AXLOGE("PZGGamefieldPlatforms::init: gameplayInfo is null");
        return false;
    }
    AXLOGI("PZGGamefieldPlatforms::init: Got gameplay info");

    if (!gameplayInfo->platform1Art) {
        AXLOGE("PZGGamefieldPlatforms::init: platform1Art is null");
        return false;
    }

    if (!gameplayInfo->platform1Art->key) {
        AXLOGE("PZGGamefieldPlatforms::init: platform1Art->key is null");
        return false;
    }

    AXLOGI("PZGGamefieldPlatforms::init: Platform key: {}", gameplayInfo->platform1Art->key->getCString());

    // Non-critical resources - log warnings but continue
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    if (!obstacles_info) {
        AXLOGW("PZGGamefieldPlatforms::init: obstacles_info not found");
    } else {
        AXLOGI("PZGGamefieldPlatforms::init: Got obstacles_info with {} items", obstacles_info->count());
    }

    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    if (!coins_info) {
        AXLOGW("PZGGamefieldPlatforms::init: coins_info not found");
    } else {
        AXLOGI("PZGGamefieldPlatforms::init: Got coins_info with {} items", coins_info->count());
    }

    __Array* levels = (__Array*)gsd->gameInfoResource->objectForKey( "Levels" );
    if (!levels) {
        AXLOGW("PZGGamefieldPlatforms::init: Levels not found");
    } else {
        AXLOGI("PZGGamefieldPlatforms::init: Processing {} levels", levels->count());

        for (int p = 0 ; p < kNumOFPlatforms; p++) {
            // levels_info is stored as a member; retain so it survives autorelease pool drains between restarts.
            AX_SAFE_RELEASE_NULL(levels_info[p]);
            levels_info[p] = __Array::create();
            AX_SAFE_RETAIN(levels_info[p]);

            //filtering all levels and adding only levels that ralated to curent platfroms
            for (int i=0; i < levels->count(); i++) {
                PZGGameInfoLevel *level = (PZGGameInfoLevel*)levels->objectAtIndex( i );
                if (!level) {
                    AXLOGW("PZGGamefieldPlatforms::init: Level {} is null", i);
                    continue;
                }

                if (!level->platformInfoKey) {
                    AXLOGW("PZGGamefieldPlatforms::init: Level {} platformInfoKey is null", i);
                    continue;
                }

                const char* platformKey = (gameplayInfo && gameplayInfo->platform1Art && gameplayInfo->platform1Art->key)
                    ? gameplayInfo->platform1Art->key->getCString()
                    : nullptr;
                if (!platformKey) {
                    AXLOGW("PZGGamefieldPlatforms::init: gameplayInfo platform1Art/key is null");
                    break; // can't classify levels without the platform key
                }

                if (level->platformInfoKey->compare(platformKey) == 0 && level->platformInfoIndex == p) {
                    levels_info[p]->addObject(level);
                }
            }

            AXLOGI("Platform type {}: {} levels", p, levels_info[ p ]->count());
        }
    }

    AXLOGI("PZGGamefieldPlatforms::init: Calling reset");
    reset();

    AXLOGI("PZGGamefieldPlatforms::init: Completed successfully");
    return true;
}

void PZGGamefieldPlatforms::reset(){
    length = 0;
    prefillPending = true;
}

void PZGGamefieldPlatforms::addPlatform(){
    
    //Remove last platform from array
    int i = kMaxPlatfroms-1;
    if ( platforms[ i ] ) {
        platforms[ i ]->destroyPhysics(world);
        platforms[ i ]->removeFromParentAndCleanup( true );
        platforms[ i ] = NULL;
    }
    
    for (int i = kMaxPlatfroms-1; i > 0; i--){
        platforms[ i ] = platforms[ i-1 ];
    }
    
    PZGPlatfrom *platform = NULL;
    
    Sprite *sprite=NULL;
    int platform_type_id = rand() % kNumOFPlatforms;
    if (isSlowMoveMode) {
        platform_type_id = 0;
    }
    
    if( platform_type_id == 0 ){
        platform = PZGPlatfrom::create(gameplayInfo->platform1Art, world);
    }
    else if(platform_type_id == 1){
        platform = PZGPlatfrom::create(gameplayInfo->platform2Art, world);
    }
    else if(platform_type_id == 2){
        platform = PZGPlatfrom::create(gameplayInfo->platform3Art, world);
    }
    
    if (platform == NULL) {
        return;
    }
    
    sprite = platform->sprite;
    
    if (sprite == NULL) {
        return;
    }
    
    sprite->setAnchorPoint( ccp(0, 0));
    if (isSlowMoveMode) {
        sprite->setPosition( ccp( length, -4) );
    }
    else{
        sprite->setPosition( ccp( length, -sprite->getContentSize().height * CCRANDOM_0_1()*0.3*gameplayInfo->platfromsHeightSpread - 4)  );
    }

    length += sprite->getContentSize().width - 4 * CC_CONTENT_SCALE_FACTOR();
    length += (10 + 10*CCRANDOM_0_1()) * gameplayInfo->platfromsBetweenDistance * kDeviceScale();
    
    Vec2 cp0;
    Vec2 cp1;
    
    if( platform_type_id == 0 ){
        cp0 = gameplayInfo->platform1Cp[0];
        cp1 = gameplayInfo->platform1Cp[1];
    }
    else if(platform_type_id == 1){
        cp0 = gameplayInfo->platform2Cp[0];
        cp1 = gameplayInfo->platform2Cp[1];
    }
    else if(platform_type_id == 2){
        cp0 = gameplayInfo->platform3Cp[0];
        cp1 = gameplayInfo->platform3Cp[1];
    }
    
    // Box2D SetAsBox expects half-extents. The plist cp points define the segment endpoints in sprite-local pixels.
    const Vec2 center = (cp0 + cp1) * 0.5f;
    const float halfWidth = std::fabs(cp1.x - cp0.x) * 0.5f;

    platform->collisionPosition = center;
    platform->width = halfWidth;
    
    platform->initPhysics(world);
    
    platforms[ 0 ] = platform;
    
    this->addChild(platform);
    
    if (gameplayInfo->randomLevelGeneration == false) {
        addLevel( platform_type_id );
    }

}

void PZGGamefieldPlatforms::destoryPhysics(){
    
    for (int i = 0; i < kMaxPlatfroms; i++){
//        if ( platforms_body[ i ] ) {
//            world->DestroyBody( platforms_body[ i ] );
//            platforms_body[ i ] = NULL;
//        }
        
        if ( platforms[ i ] ) {
            platforms[ i ]->destroyPhysics(world);
            platforms[ i ]->removeFromParentAndCleanup( true );
            platforms[ i ] = NULL;
        }
        
    }
    
    for (int i = 0; i < kMaxBasicRunnerObsctales; i++) {
        if (obstacles[ i ]) {
            obstacles[ i ]->removeFromParentAndCleanup( true );
            obstacles[ i ]->destroyPhysics( world );
            obstacles[ i ] = NULL;
        }
    }
    
    for (int i = 0; i < kMaxBasicRunnerCoins; i++) {
        if (coins[ i ]) {
            coins[ i ]->removeFromParentAndCleanup( true );
            coins[ i ]->destroyPhysics( world );
            coins[ i ] = NULL;
        }
    }
}

int PZGGamefieldPlatforms::getCoinIndex( PZGGameInfoLevelItem *item ){
    for (int i=0; i < coins_info->count(); i++) {
        PZGArtCoins *coin = ( PZGArtCoins* )coins_info->objectAtIndex( i );
        if (coin->index == item->infoObjIndex) {
            return i;
        }
    }
    
    return 0;
}

int PZGGamefieldPlatforms::getObstacleIndex( PZGGameInfoLevelItem *item ){
    for (int i=0; i < obstacles_info->count(); i++) {
        PZGArtObstacle *obsacle = ( PZGArtObstacle* )obstacles_info->objectAtIndex( i );
        if (obsacle->index == item->infoObjIndex) {
            return i;
        }
    }
    
    return 0;
}

PZGGameInfoLevel* PZGGamefieldPlatforms::getLevelInfo(float distance, int platform_type_id){

    if (!levels_info[ platform_type_id ]) {
        return NULL;
    }
    
    if (levels_info[ platform_type_id ]->count() == 0) {
        return NULL;
    }
    
    unsigned int pool[ 64 ];
    
    int n=0;
    for (int i=0; i<levels_info[ platform_type_id ]->count(); i++) {
        PZGGameInfoLevel *levelInfo =  (PZGGameInfoLevel*)levels_info[ platform_type_id ]->objectAtIndex( i );
        
        if( levelInfo->startDistance < distance && levelInfo->endDistance > distance){
            pool[ n ] = i;
            n++;
        }
    }
    
    if (n == 0) {
        return NULL;
    }
    
    int r = rand() % n;
    
    return (PZGGameInfoLevel*)levels_info[ platform_type_id ]->objectAtIndex( pool[ r ] );

}

void PZGGamefieldPlatforms::addLevel( int platform_type_id ){

    PZGGameInfoLevel *levelInfo = getLevelInfo(distance, platform_type_id);

    if (levelInfo == NULL) {
        return;
    }

    if (!levelInfo->objects) {
        AXLOGW("PZGGamefieldPlatforms::addLevel: levelInfo->objects is null");
        return;
    }

    Vec2 spritePosition = platforms[ 0 ]->sprite->getPosition();
    std::vector<PZGObstacle*> pendingObstacles;

    // Coin density: the chunk's coin items are authored in groups; a contiguous run of `kCoinShapeMinCoins` or more is a
    // deliberate shape (BOOST / BIKE / arrow ...) and is kept whole, while loose single coins are thinned out so the
    // track stops looking like a carpet of gold.
    const int itemCount = levelInfo->objects->count();
    std::vector<int> coinRunLen(itemCount, 0);
    for (int i = 0; i < itemCount; ) {
        auto* it = (PZGGameInfoLevelItem*)levelInfo->objects->objectAtIndex(i);
        const bool isCoin = it && it->infoObjKey && it->infoObjKey->compare("Coins") == 0;
        if (!isCoin) { ++i; continue; }
        int j = i;
        while (j < itemCount) {
            auto* jt = (PZGGameInfoLevelItem*)levelInfo->objects->objectAtIndex(j);
            if (!jt || !jt->infoObjKey || jt->infoObjKey->compare("Coins") != 0) break;
            ++j;
        }
        for (int k = i; k < j; ++k) coinRunLen[k] = j - i;
        i = j;
    }

    for (int i=0;  i < levelInfo->objects->count(); i++) {
        PZGGameInfoLevelItem *item = (PZGGameInfoLevelItem*)levelInfo->objects->objectAtIndex( i );
        if (!item || !item->infoObjKey) {
            AXLOGW("PZGGamefieldPlatforms::addLevel: Item {} is null or has null infoObjKey", i);
            continue;
        }

        if (item->infoObjKey->compare( "Coins" ) == 0) {
            if (!coins_info) {
                AXLOGW("PZGGamefieldPlatforms::addLevel: coins_info is null, skipping coin");
                continue;
            }

            // Thin out loose coins (keep every shape intact).
            if (coinRunLen[i] < std::max(1, PZCfg::kCoinShapeMinCoins) &&
                CCRANDOM_0_1() >= std::max(0.f, std::min(1.f, PZCfg::kCoinDensity))) {
                continue;
            }

            int index = getCoinIndex( item );
            if (index < 0 || index >= (int)coins_info->count()) {
                AXLOGW("PZGGamefieldPlatforms::addLevel: Coin index {} out of range [0, {})", index, coins_info->count());
                continue;
            }

            PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( index ), world);
            if (coin == NULL) {
                AXLOGW("PZGGamefieldPlatforms::addLevel: Failed to create coin at index {}", index);
                continue;
            }
            
            coin->setPosition( item->position );
            coin->setPosition( ccp(spritePosition.x + coin->getPosition().x * kDeviceScale()*2,
                                   spritePosition.y + coin->getPosition().y * kDeviceScale()*2) );
            coin->setRotation( -item->rotation );
            coin->setScaleX( item->scale.x );
            coin->setScaleY( item->scale.y );
            
            registerCoin( coin );
        }
        else if( item->infoObjKey->compare( "Obstacles" ) == 0){
            if (!obstacles_info) {
                AXLOGW("PZGGamefieldPlatforms::addLevel: obstacles_info is null, skipping obstacle");
                continue;
            }

            int index = getObstacleIndex( item );
            if (index < 0 || index >= (int)obstacles_info->count()) {
                AXLOGW("PZGGamefieldPlatforms::addLevel: Obstacle index {} out of range [0, {})", index, obstacles_info->count());
                continue;
            }

            PZGObstacle *obstacle = PZGObstacle::create( (PZGArtObstacle*)obstacles_info->objectAtIndex( index ), world);
            if (obstacle == NULL) {
                AXLOGW("PZGGamefieldPlatforms::addLevel: Failed to create obstacle at index {}", index);
                continue;
            }
            obstacle->setPosition( item->position );
            obstacle->setPosition( ccp(spritePosition.x + obstacle->getPosition().x * kDeviceScale()*2,
                                       spritePosition.y + obstacle->getPosition().y * kDeviceScale()*2) );
            obstacle->setRotation( -item->rotation );
            obstacle->setScaleX( item->scale.x );
            obstacle->setScaleY( item->scale.y );
            
            pendingObstacles.push_back( obstacle );
        }
    }

    finalizeChunk( pendingObstacles, platforms[ 0 ] );
}

void PZGGamefieldPlatforms::addCoins( int typeId ){

    // Don't spawn too close to the visible area. The old constant (1200) assumed a ~960-wide world.
    // With `ResolutionPolicy::FIXED_HEIGHT`, the visible width varies by aspect ratio, so scale it.
    const Size screen = Director::getInstance()->getWinSize();
    if (length < screen.width * 1.25f){
        return;
    }

    if (!coins_info) {
        AXLOGW("PZGGamefieldPlatforms::addCoins: coins_info is null");
        return;
    }

    if (typeId < 0 || typeId >= (int)coins_info->count()) {
        AXLOGW("PZGGamefieldPlatforms::addCoins: typeId {} out of range [0, {})", typeId, coins_info->count());
        return;
    }

    // (screen already computed above)
    Vec2 spritePosition = platforms[ 0 ]->sprite->getPosition();
    Vec2 collisionPosition = platforms[ 0 ]->collisionPosition;// platfroms_collision[ 0 ];
    float collisionWidth = platforms[ 0 ]->width;// platforms_width[ 0 ];

    PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( typeId ), world);
    if (coin == NULL) {
        AXLOGW("PZGGamefieldPlatforms::addCoins: Failed to create coin at typeId {}", typeId);
        return;
    }

    const float coinW = coin->sprite->getContentSize().width;
    const float coinH = coin->sprite->getContentSize().height;

    auto blockerAt = [&](float x, float y) -> PZGObstacle* {
        const Rect rn = RectMake(x - coinW * 0.5f, y - coinH * 0.5f, coinW, coinH);
        for (int i = 0; i < kMaxBasicRunnerObsctales; i++) {
            PZGObstacle* o = obstacles[i];
            if (!o || !o->sprite) continue;
            const Size os = o->sprite->getContentSize();
            const Rect r = RectMake(o->getPosition().x - os.width * o->obstacle_scale * 0.5f,
                                    o->getPosition().y - os.height * o->obstacle_scale * 0.5f,
                                    os.width * o->obstacle_scale,
                                    os.height * o->obstacle_scale);
            if (r.intersectsRect(rn)) return o;
        }
        return nullptr;
    };

    // The old code gave up (and leaked the coin) whenever the single random spot landed on an obstacle. Obstacle
    // density ramps up with distance, so by ~500m almost every spot was blocked and the coins simply stopped
    // appearing. Try a handful of spots along the platform and lift the coin into a reachable arc over the hazard.
    const float baseX = spritePosition.x + collisionPosition.x * kDeviceScale();
    const float baseY = spritePosition.y + collisionPosition.y * kDeviceScale() + coinH * 0.5f;
    const float span = std::max(collisionWidth * kDeviceScale(), 60.f);

    bool placed = false;
    Vec2 spot(0.f, 0.f);
    for (int attempt = 0; attempt < 8 && !placed; ++attempt) {
        const float x = baseX + CCRANDOM_MINUS1_1() * span;
        if (x < screen.width * 1.08f) continue;    // keep the spawn out of the visible area

        float y = baseY;
        if (PZGObstacle* blocker = blockerAt(x, y)) {
            const float obsH = blocker->sprite->getContentSize().height * blocker->obstacle_scale;
            y = blocker->getPosition().y + obsH * 0.5f + coinH + 18.f * kDeviceScale();
            if (blockerAt(x, y)) continue;
        }
        if (y > screen.height - 90.f) continue;

        spot = Vec2(x, y);
        placed = true;
    }

    if (!placed) {
        // Nothing free on this platform: drop the coin cleanly instead of leaking a physics body.
        coin->destroyPhysics(world);
        coin->release();
        return;
    }

    coin->setPosition(ccp(spot.x, spot.y));

            registerCoin( coin );
}
void PZGGamefieldPlatforms::addObstacle( int typeId ){

    // Don't spawn too close to the visible area. The old constant (1200) assumed a ~960-wide world.
    const Size screen = Director::getInstance()->getWinSize();
    if (length < screen.width * 1.25f){
        return;
    }

    if (!obstacles_info) {
        AXLOGW("PZGGamefieldPlatforms::addObstacle: obstacles_info is null");
        return;
    }

    if (typeId < 0 || typeId >= (int)obstacles_info->count()) {
        AXLOGW("PZGGamefieldPlatforms::addObstacle: typeId {} out of range [0, {})", typeId, obstacles_info->count());
        return;
    }

    Vec2 spritePosition = platforms[ 0 ]->sprite->getPosition();
    Vec2 collisionPosition = platforms[ 0 ]->collisionPosition;// platfroms_collision[ 0 ];
    float collisionWidth = platforms[ 0 ]->width;// platforms_width[ 0 ];

    PZGArtObstacle *obstInfo = (PZGArtObstacle*)obstacles_info->objectAtIndex( typeId );
    if (!obstInfo) {
        AXLOGW("PZGGamefieldPlatforms::addObstacle: obstInfo is null at typeId {}", typeId);
        return;
    }

    PZGObstacle *obstacle = PZGObstacle::create( obstInfo, world);

        if (obstacle == NULL) {
            return;
        }
        
        if (isSlowMoveMode) {
            obstacle->setPosition( ccp(spritePosition.x + collisionPosition.x*kDeviceScale() + CCRANDOM_MINUS1_1()*collisionWidth*kDeviceScale(),
                                       spritePosition.y + collisionPosition.y*kDeviceScale() + CCRANDOM_0_1() * 70*CC_CONTENT_SCALE_FACTOR() ) );
        }
        else{
            obstacle->setPosition( ccp(spritePosition.x + collisionPosition.x*kDeviceScale() + CCRANDOM_MINUS1_1()*collisionWidth*0.65*kDeviceScale(),
                                       spritePosition.y + collisionPosition.y*kDeviceScale() + obstacle->sprite->getContentSize().height*0.5 * obstacle->obstacle_scale) );
        }
        obstacle->velocityLinear = ccp(0,0);
        
        if (obstacle->getPosition().x < screen.width*1.1) {
            return;
        }
        
        Rect rn = RectMake(obstacle->getPosition().x - obstacle->sprite->getContentSize().width * obstacle->obstacle_scale * 0.5 * 2,
                               obstacle->getPosition().y - obstacle->sprite->getContentSize().height * obstacle->obstacle_scale * 0.5,
                               obstacle->sprite->getContentSize().width * obstacle->obstacle_scale * 2,
                               obstacle->sprite->getContentSize().height * obstacle->obstacle_scale);
        
        //check for obstacle overlap
        for (int i=0; i < kMaxBasicRunnerObsctales; i++) {
            if (obstacles[ i ]) {
                Rect r = RectMake(obstacles[ i ]->getPosition().x - obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->getPosition().y - obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale,
                                      obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale);
                
                if (r.intersectsRect( rn )) {
                    return;
                }
            }
        }
        
        // check for overlap with coins
        for (int i=0; i < kMaxBasicRunnerCoins; i++) {
            if (coins[ i ]) {
                Rect r = RectMake(coins[ i ]->getPosition().x - coins[ i ]->sprite->getContentSize().width * 0.5,
                                      coins[ i ]->getPosition().y - coins[ i ]->sprite->getContentSize().height * 0.5,
                                      coins[ i ]->sprite->getContentSize().width,
                                      coins[ i ]->sprite->getContentSize().height);
                if (r.intersectsRect( rn )) {
                    return;
                }
            }
        }
        
    registerObstacle( obstacle, 32 - (int)(obstacles_info->count() - typeId) );
}

void PZGGamefieldPlatforms::update(float dt){
    // Initial fill must happen only after `world` is assigned by the owning gamefield layer.
    // Never create bodies while Box2D is locked (during Step / contact callbacks).
    if (prefillPending) {
        if (world && !world->IsLocked()) {
            const Size screen = Director::getInstance()->getWinSize();
            const float minAhead = screen.width * 2.2f;
            while (length < minAhead) {
                addPlatform();
            }
            prefillPending = false;
        }
        // If world isn't ready yet, skip work this frame.
    }

    float s =  dt*speed*32*kDeviceScale()*2;

    for (int i = 0; i < kMaxPlatfroms; i++){
        if (platforms[ i ]) {
            
            Sprite *sprite = platforms[i]->sprite;
            
            //platforms[ i ]->setPosition( ccp(platforms[ i ]->getPosition().x - s, platforms[ i ]->getPosition().y) );
            
            sprite->setPosition( ccp(sprite->getPosition().x - s, sprite->getPosition().y) );
            
            //platforms_body[ i ]->SetTransform(b2Vec2(platforms_body[ i ]->GetPosition().x - s*kPhysicsWorldScale(), platforms_body[ i ]->GetPosition().y) , 0);
            
            b2Body* body = platforms[ i ]->body;
            body->SetTransform(b2Vec2(body->GetPosition().x - s*kPhysicsWorldScale(), body->GetPosition().y) , 0);
        }
    }
    
    //UPDATE SPAWN COUNTERS
    if (obstacles_info && gameplayInfo->randomLevelGeneration) {
        for (int i=0; i < obstacles_info->count(); i++) {
            PZGArtObstacle *info = (PZGArtObstacle*)obstacles_info->objectAtIndex( i );
            obstacles_spawn_counter[ i ] += dt;
            
            if (info->comingRatio <= obstacles_spawn_counter[ i ] && info->comingRatio > 0) {
                addObstacle( i );
                obstacles_spawn_counter[ i ] = CCRANDOM_0_1() * (info->comingRatio*0.5);
            }
        }
    }
    
    if (coins_info && gameplayInfo->randomLevelGeneration) {
        // Half the spawn rate (tuning/CoinDensity): random mode used to drop a coin on every comingRatio tick.
        const float coinRate = std::max(0.05f, std::min(1.f, PZCfg::kCoinDensity));
        for (int i=0; i < coins_info->count(); i++) {
            PZGArtCoins *info = (PZGArtCoins*)coins_info->objectAtIndex( i );
            coins_spawn_counter[ i ] += dt * coinRate;
            
            if (info->comingRatio <= coins_spawn_counter[ i ] && info->comingRatio > 0) {
                addCoins( i );
                coins_spawn_counter[ i ] = CCRANDOM_0_1() * (info->comingRatio*0.5);
            }
        }
    }
    
    //UPDATE OBJECT COUNTERS
    for (int i = 0; i < kMaxBasicRunnerObsctales; i++) {
        if (obstacles[ i ]) {
            obstacles[ i ]->setPosition( ccp(obstacles[ i ]->getPosition().x - s, obstacles[ i ]->getPosition().y) );
            obstacles[ i ]->update( dt );
            
            // Passed objects are culled here (they used to be evicted by accident when the arrays overflowed).
            if (obstacles[ i ]->deleteReady || obstacles[ i ]->getPosition().x < -420.0f * kDeviceScale()) {
                obstacles[ i ]->removeFromParentAndCleanup( true );
                obstacles[ i ]->destroyPhysics( world );
                obstacles[ i ] = NULL;
            }
        }
    }
    
    
    for (int i = 0; i < kMaxBasicRunnerCoins; i++) {
        if (coins[ i ]) {
            coins[ i ]->setPosition( ccp(coins[ i ]->getPosition().x - s, coins[ i ]->getPosition().y) );
            coins[ i ]->update( dt );
            
            if (coins[ i ]->deleteReady || coins[ i ]->getPosition().x < -260.0f * kDeviceScale()) {
                coins[ i ]->removeFromParentAndCleanup( true );
                coins[ i ]->destroyPhysics( world );
                coins[ i ] = NULL;
            }
        }
    }

    length -= s;

    // Keep enough platforms ahead so the next one is already spawned before it enters view.
    // With `ResolutionPolicy::FIXED_HEIGHT`, `screen.width` can exceed the old hardcoded 1200 threshold.
    const Size screen = Director::getInstance()->getWinSize();
    const float minAhead = screen.width * 2.2f;
    if (world && !world->IsLocked()) {
        while (length < minAhead) {
            addPlatform();
        }
    }

}

// ---------------------------------------------------------------------------------------------
// Meta-feature helpers (revive / turbo landing)
// ---------------------------------------------------------------------------------------------

bool PZGGamefieldPlatforms::groundYAt(float x, float& outY) const {
    const float k = kDeviceScale();
    const float inv = 1.0f / kPhysicsWorldScale();
    bool found = false;
    float best = 0.0f;
    for (int i = 0; i < kMaxPlatfroms; ++i) {
        const PZGPlatfrom* p = platforms[i];
        if (!p || !p->body) continue;
        const float cx = p->body->GetPosition().x * inv;
        const float hw = p->width * k;
        if (x < cx - hw || x > cx + hw) continue;
        const float top = p->body->GetPosition().y * inv + 20.0f * k;
        if (!found || top > best) {
            best = top;
            found = true;
        }
    }
    outY = best;
    return found;
}

// Finds the platform a rescued rider should land on and how far the world must slide left (outDx >= 0) to put its start
// under the rider. Nothing moves here (see shiftWorld), so the caller can animate the slide.
bool PZGGamefieldPlatforms::findReviveTarget(float x, float& outGroundY, float& outDx) {
    const float k = kDeviceScale();
    const float inv = 1.0f / kPhysicsWorldScale();

    // Nearest platform (smallest left edge) that still gives some runway ahead of the rider.
    // Prefer a platform with real runway (the rider must get a chance to react before the next gap); only when none
    // is long enough, take any platform that is still under/ahead of the rider.
    const PZGPlatfrom* best = nullptr;
    float bestLeft = 0.0f;
    const float runways[2] = {430.0f * k, 140.0f * k};
    for (int pass = 0; pass < 2 && !best; ++pass) {
        for (int i = 0; i < kMaxPlatfroms; ++i) {
            const PZGPlatfrom* p = platforms[i];
            if (!p || !p->body) continue;
            const float cx = p->body->GetPosition().x * inv;
            const float hw = p->width * k;
            if (cx + hw < x + runways[pass]) continue;
            if (!best || cx - hw < bestLeft) {
                best = p;
                bestLeft = cx - hw;
            }
        }
    }
    if (!best) return false;

    const float want = x - 70.0f * k;
    outDx = (bestLeft <= want) ? 0.0f : (bestLeft - want);
    outGroundY = best->body->GetPosition().y * inv + 20.0f * k;
    return true;
}

// Slides everything in the world left by dx (used by the animated rescue; may be called every frame with small steps).
void PZGGamefieldPlatforms::shiftWorld(float dx) {
    if (dx == 0.0f) return;
    for (int i = 0; i < kMaxPlatfroms; ++i) {
        PZGPlatfrom* p = platforms[i];
        if (!p || !p->body || !p->sprite) continue;
        p->sprite->setPosition(ccp(p->sprite->getPosition().x - dx, p->sprite->getPosition().y));
        p->body->SetTransform(b2Vec2(p->body->GetPosition().x - dx * kPhysicsWorldScale(), p->body->GetPosition().y), 0);
    }
    for (int i = 0; i < kMaxBasicRunnerObsctales; ++i) {
        if (obstacles[i]) obstacles[i]->setPosition(ccp(obstacles[i]->getPosition().x - dx, obstacles[i]->getPosition().y));
    }
    for (int i = 0; i < kMaxBasicRunnerCoins; ++i) {
        if (coins[i]) coins[i]->setPosition(ccp(coins[i]->getPosition().x - dx, coins[i]->getPosition().y));
    }
    length -= dx;
}

// Fair restart: wipe hazards around x (removed by the regular update loop).
void PZGGamefieldPlatforms::clearHazardsNear(float x) {
    const float k = kDeviceScale();
    for (int i = 0; i < kMaxBasicRunnerObsctales; ++i) {
        PZGObstacle* o = obstacles[i];
        if (!o || o->dead) continue;
        const float ox = o->getPosition().x;
        if (ox > x - 160.0f * k && ox < x + 520.0f * k) {
            // Flag only; the regular update loop removes it (safe: keeps sprite pointers valid this frame).
            o->dead = true;
            o->deleteReady = true;
        }
    }
}

// Instant version (kept for callers that want the old behaviour).
bool PZGGamefieldPlatforms::alignForRevive(float x, float& outGroundY) {
    float dx = 0.0f;
    if (!findReviveTarget(x, outGroundY, dx)) return false;
    shiftWorld(dx);
    clearHazardsNear(x);
    return true;
}


// ---------------------------------------------------------------------------------------------
// Obstacle placement rules + guide coins (see Features/PZGameConfig.h, section "obstacle fairness")
// ---------------------------------------------------------------------------------------------

void PZGGamefieldPlatforms::registerObstacle( PZGObstacle* obstacle, int zOrder ){
    if (zOrder == INT_MIN) {
        zOrder = 32 - (obstacles_info ? (int)obstacles_info->count() : 0);
    }
    int slot = -1;
    for (int i = 0; i < kMaxBasicRunnerObsctales; ++i) {
        if (!obstacles[ i ]) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        // Truly full (should not happen with the capacity above): drop the object furthest behind the rider.
        float leftmost = 1.0e9f;
        for (int i = 0; i < kMaxBasicRunnerObsctales; ++i) {
            if (obstacles[ i ] && obstacles[ i ]->getPosition().x < leftmost) {
                leftmost = obstacles[ i ]->getPosition().x;
                slot = i;
            }
        }
        obstacles[ slot ]->destroyPhysics( world );
        this->removeChild( obstacles[ slot ], true );
        obstacles[ slot ] = NULL;
    }
    this->addChild( obstacle, zOrder );
    obstacles[ slot ] = obstacle;
}

void PZGGamefieldPlatforms::registerCoin( PZGCoin* coin ){
    int slot = -1;
    for (int i = 0; i < kMaxBasicRunnerCoins; ++i) {
        if (!coins[ i ]) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        float leftmost = 1.0e9f;
        for (int i = 0; i < kMaxBasicRunnerCoins; ++i) {
            if (coins[ i ] && coins[ i ]->getPosition().x < leftmost) {
                leftmost = coins[ i ]->getPosition().x;
                slot = i;
            }
        }
        coins[ slot ]->destroyPhysics( world );
        this->removeChild( coins[ slot ], true );
        coins[ slot ] = NULL;
    }
    this->addChild( coin, 33 );
    coins[ slot ] = coin;
}

void PZGGamefieldPlatforms::addCoinArc( const Vec2& from, const Vec2& to, float peakRise, int count, const std::vector<Rect>& avoid ){
    if (!coins_info || coins_info->count() == 0 || count < 2) {
        return;
    }
    // Guide coins follow the same density rule, but never drop below 3 so the jump hint stays readable.
    count = std::max(3, static_cast<int>(std::lround(count * std::max(0.05f, std::min(1.f, PZCfg::kCoinDensity)))));
    for (int i = 0; i < count; ++i) {
        const float t = (float)i / (float)(count - 1);
        Vec2 centre = from + (to - from) * t;
        centre.y += peakRise * 4.0f * t * (1.0f - t);

        bool blocked = false;
        for (const Rect& r : avoid) {
            if (r.containsPoint( centre )) {
                blocked = true;
                break;
            }
        }
        if (blocked) {
            continue;
        }

        PZGCoin* coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( 0 ), world );
        if (!coin || !coin->sprite) {
            continue;
        }
        const Size cs = coin->sprite->getContentSize();
        coin->setPosition( centre - (coin->sprite->getPosition() + ccp(cs.width * coin->scale * 0.5f, cs.height * coin->scale * 0.5f)) );
        registerCoin( coin );
    }
}

void PZGGamefieldPlatforms::finalizeChunk( std::vector<PZGObstacle*>& pending, PZGPlatfrom* platform ){
    const float k = kDeviceScale();
    const float inv = 1.0f / kPhysicsWorldScale();

    if (!platform || !platform->body) {
        for (PZGObstacle* o : pending) registerObstacle( o );
        return;
    }

    const float cx = platform->body->GetPosition().x * inv;
    const float hw = platform->width * k;
    const float surfaceL = cx - hw;
    const float surfaceR = cx + hw;
    const float groundY = platform->body->GetPosition().y * inv + 20.0f * k;

    std::vector<PZGObstacle*> lethal;
    for (PZGObstacle* o : pending) {
        if (!o->destroyCharacter) {
            registerObstacle( o );             // ramps / rails: always kept
        } else {
            lethal.push_back( o );
        }
    }

    auto discard = [this]( PZGObstacle* o ) {
        o->destroyPhysics( world );
        o->release();                          // never added to the scene graph: drop the creation reference
    };

    if (!PZCfg::kEnableFairPlacement) {
        for (PZGObstacle* o : lethal) registerObstacle( o );
        return;
    }

    // 1. Reject anything that can't be cleared or that floats outside the platform surface.
    std::vector<PZGObstacle*> candidates;
    for (PZGObstacle* o : lethal) {
        const Rect b = o->worldBox();
        const bool tooTall = (b.getMaxY() - groundY) > riderApex * 0.88f;
        const bool outside = b.getMidX() < surfaceL || b.getMidX() > surfaceR;
        const bool inLanding = b.getMinX() < surfaceL + PZCfg::kSafeLanding * k;
        const bool inTakeoff = b.getMaxX() > surfaceR - PZCfg::kSafeTakeoff * k;
        if (tooTall || outside || inLanding || inTakeoff) {
            discard( o );
        } else {
            candidates.push_back( o );
        }
    }

    // 2. Group into clusters by horizontal proximity.
    std::sort( candidates.begin(), candidates.end(), []( PZGObstacle* a, PZGObstacle* b ) {
        return a->worldBox().getMinX() < b->worldBox().getMinX();
    });
    struct Cluster { std::vector<PZGObstacle*> items; float minX; float maxX; float topY; };
    std::vector<Cluster> clusters;
    for (PZGObstacle* o : candidates) {
        const Rect b = o->worldBox();
        if (!clusters.empty() && b.getMinX() - clusters.back().maxX < 90.0f * k) {
            Cluster& c = clusters.back();
            c.items.push_back( o );
            c.maxX = std::max( c.maxX, b.getMaxX() );
            c.topY = std::max( c.topY, b.getMaxY() );
        } else {
            clusters.push_back( { { o }, b.getMinX(), b.getMaxX(), b.getMaxY() } );
        }
    }

    // 3. Density ramp (gentler start), width limit and spacing between clusters.
    const float density = std::min( 1.0f, PZCfg::kDensityStart +
                          (1.0f - PZCfg::kDensityStart) * std::max( 0.0f, distance ) / std::max( 1.0f, PZCfg::kDensityFullDistance ) );
    float lastMaxX = -1.0e9f;
    std::vector<Rect> kept;
    std::vector<std::pair<float,float>> spans;      // x-range of every hazard cluster that survived
    for (Cluster& c : clusters) {
        // too wide to clear in one jump: shed obstacles from the right until it fits
        while (c.items.size() > 1 && (c.maxX - c.minX) > PZCfg::kClusterMaxWidth * k) {
            PZGObstacle* last = c.items.back();
            c.items.pop_back();
            discard( last );
            c.maxX = c.minX;
            for (PZGObstacle* o : c.items) c.maxX = std::max( c.maxX, o->worldBox().getMaxX() );
        }
        const bool thinned = CCRANDOM_0_1() > density;
        const bool tooClose = (c.minX - lastMaxX) < PZCfg::kClusterGap * k;
        if (thinned || tooClose) {
            for (PZGObstacle* o : c.items) discard( o );
            continue;
        }
        lastMaxX = c.maxX;
        spans.push_back( { c.minX, c.maxX } );
        for (PZGObstacle* o : c.items) {
            const Rect b = o->worldBox();
            kept.push_back( Rect( b.getMinX() - 8.0f, b.getMinY() - 8.0f, b.size.width + 16.0f, b.size.height + 16.0f ) );
            registerObstacle( o );
        }

        // 4. Guide coins arching over the cluster (a visible hint of where to jump).
        if (PZCfg::kEnableCoinArcs) {
            const float rise = std::min( riderApex * 0.82f, std::max( PZCfg::kArcHeight * k, (c.topY - groundY) + 60.0f * k ) );
            addCoinArc( ccp( c.minX - 80.0f * k, groundY + 30.0f * k ), ccp( c.maxX + 80.0f * k, groundY + 30.0f * k ),
                        rise, PZCfg::kArcCoinCount, kept );
        }
    }

    // 4b. Hazard budget. Chunks are hand-authored and the fairness rules above remove anything unjumpable, so a platform
    // can end up with no hazard at all. The budget guarantees a minimum number of hazard clusters per platform that
    // grows with distance, placed under the same rules (inside the safe zones, spaced, jumpable) and given guide coins.
    if (PZCfg::kEnableHazardBudget && obstacles_info && obstacles_info->count() > 0) {
        const int target = distance < PZCfg::kBudget1Distance ? 0
                         : distance < PZCfg::kBudget2Distance ? 1
                         : distance < PZCfg::kBudget3Distance ? 2 : 3;
        std::vector<int> candidates;
        for (int i = 0; i < (int)obstacles_info->count(); ++i) {
            auto* info = (PZGArtObstacle*)obstacles_info->objectAtIndex( i );
            if (info && info->destroyCharacter && !info->collideCharacter) candidates.push_back( i );
        }
        int have = (int)spans.size();
        int attempts = 0;
        const float lo = surfaceL + (PZCfg::kSafeLanding + 70.0f) * k;
        const float hi = surfaceR - (PZCfg::kSafeTakeoff + 70.0f) * k;
        while (have < target && attempts < 10 && !candidates.empty() && hi > lo) {
            ++attempts;
            const float x = lo + CCRANDOM_0_1() * (hi - lo);
            bool clash = false;
            for (const auto& sp : spans) {
                if (x > sp.first - PZCfg::kClusterGap * k * 0.7f && x < sp.second + PZCfg::kClusterGap * k * 0.7f) {
                    clash = true;
                    break;
                }
            }
            if (clash) continue;

            auto* info = (PZGArtObstacle*)obstacles_info->objectAtIndex( candidates[ rand() % candidates.size() ] );
            PZGObstacle* o = PZGObstacle::create( info, world );
            if (!o) continue;
            o->velocityLinear = ccp(0, 0);
            o->setPosition( ccp( x, platform->body->GetPosition().y * inv + o->sprite->getContentSize().height * 0.5f * o->obstacle_scale ) );
            const Rect b = o->worldBox();
            if ((b.getMaxY() - groundY) > riderApex * 0.8f || b.size.width > PZCfg::kClusterMaxWidth * k) {
                discard( o );
                continue;
            }
            spans.push_back( { b.getMinX(), b.getMaxX() } );
            kept.push_back( Rect( b.getMinX() - 8.0f, b.getMinY() - 8.0f, b.size.width + 16.0f, b.size.height + 16.0f ) );
            registerObstacle( o );
            ++have;
            if (PZCfg::kEnableCoinArcs) {
                const float rise = std::min( riderApex * 0.82f, std::max( PZCfg::kArcHeight * k, (b.getMaxY() - groundY) + 60.0f * k ) );
                addCoinArc( ccp( b.getMinX() - 80.0f * k, groundY + 30.0f * k ), ccp( b.getMaxX() + 80.0f * k, groundY + 30.0f * k ),
                            rise, PZCfg::kArcCoinCount, kept );
            }
        }
    }

    // 5. Guide coins across the gap that follows this platform.
    if (PZCfg::kEnableCoinArcs && distance > 20.0f && CCRANDOM_0_1() < 0.7f) {
        const float gapStart = surfaceR;
        const float gapEnd = platform->sprite->getPosition().x + (platform->sprite->getContentSize().width - 4.0f) + 220.0f * k;
        if (gapEnd > gapStart) {
            addCoinArc( ccp( gapStart - 70.0f * k, groundY + 30.0f * k ), ccp( gapEnd + 70.0f * k, groundY + 30.0f * k ),
                        std::min( riderApex * 0.7f, 150.0f * k ), PZCfg::kArcCoinCount, kept );
        }
    }
}
