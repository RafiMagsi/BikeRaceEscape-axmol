//
//  PZGGamefieldJumpingPlatforms.cpp
//  ProjectMayhem
//
//  Created by Amit Srivastava on 08/05/13.
//
//

#include "PZLegacyCompat.h"
#include "PZGGamefieldJumpingPlatforms.h"
#include "constants.h"
#include <cstring>

USING_NS_AX;

PZGGamefieldJumpingPlatforms::PZGGamefieldJumpingPlatforms()
    : gameplayInfo(nullptr)
    , obstacles_info(nullptr)
    , coins_info(nullptr)
    , length(0.0f) {
    std::memset(levels_info, 0, sizeof(levels_info));
    std::memset(platforms, 0, sizeof(platforms));
    std::memset(obstacles, 0, sizeof(obstacles));
    std::memset(coins, 0, sizeof(coins));
}

PZGGamefieldJumpingPlatforms::~PZGGamefieldJumpingPlatforms() {
    for (int p = 0; p < kNum_OFPlatforms; ++p) {
        AX_SAFE_RELEASE_NULL(levels_info[p]);
    }
}

Scene* PZGGamefieldJumpingPlatforms::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldJumpingPlatforms *layer = PZGGamefieldJumpingPlatforms::create();
    
	scene->addChild(layer);
	return scene;
}

bool PZGGamefieldJumpingPlatforms::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey( "GameplaySettings" );
    gameplayInfo = (array && array->count() > 0) ? (PZGGameplayJumpingGame*)array->objectAtIndex(0) : nullptr;
    if (!gameplayInfo || !gameplayInfo->platform1Art || !gameplayInfo->platform1Art->key) {
        AXLOGE("PZGGamefieldJumpingPlatforms::init: GameplaySettings/platform1Art/key missing");
        return false;
    }
    
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    
    __Array* levels = (__Array*)gsd->gameInfoResource->objectForKey( "Levels" );
    
    if (levels) {
        for (int p = 0 ; p < kNum_OFPlatforms; p++) {
            AX_SAFE_RELEASE_NULL(levels_info[p]);
            levels_info[ p ] = __Array::create();
            AX_SAFE_RETAIN(levels_info[p]);
            
            //filtering all levels and adding only levels that ralated to curent platfroms
            for (int i=0; i < levels->count(); i++) {
                PZGGameInfoLevel *level = (PZGGameInfoLevel*)levels->objectAtIndex( i );
                if (!level || !level->platformInfoKey) {
                    continue;
                }
                if (level->platformInfoKey->compare(gameplayInfo->platform1Art->key->getCString()) == 0 &&
                    level->platformInfoIndex == p) {
                    levels_info[ p ]->addObject( level );
                }
            }
            
            printf(" --- platform_type_id: %d  COUNT: %d\n", p, levels_info[ p ]->count());
        }
    }
    
    
    reset();
    
    //setTag( kPlatfromTag );
    
    return true;
}

void PZGGamefieldJumpingPlatforms::reset(){
    length = 0;
}

void PZGGamefieldJumpingPlatforms::addPlatforms(float dt) {
    
    if (platforms [0] == NULL) {
        ax::Rect rect = ax::Rect::ZERO;

        for (int pIndex = kMax_Platfroms-1; pIndex >= 0; pIndex--){
            rect = addPlatform(pIndex,dt, rect);
            
        }
        
    }
    
}

void PZGGamefieldJumpingPlatforms::scroll(float scrollDiff, float dt){
    
    
    ///here is your scroll method. here is your scrolling platforms
//    so i think this is nice place to scroll obstacel and coins too
    
    float factor=1.0*scrollDiff;
    
    
    //UPDATE OBJECT COUNTERS
    for (int i = 0; i < kMax_BasicRunnerObsctales; i++) {
        if (obstacles[ i ]) {
            
            Vec2 loc = obstacles[ i ]->getPosition();
            
            loc.y-=factor;
            
            obstacles[ i ]->setPosition(loc);
            
            obstacles[ i ]->update( dt );
            
            if (obstacles[ i ]->deleteReady) {
                obstacles[ i ]->removeFromParentAndCleanup( true );
                obstacles[ i ]->destroyPhysics( world );
                obstacles[ i ] = NULL;
            }
        }
    }
    
    
    for (int i = 0; i < kMax_BasicRunnerCoins; i++) {
        if (coins[ i ]) {
            
            Vec2 loc = coins[ i ]->getPosition();
            
            loc.y-=factor;
            
            coins[ i ]->setPosition(loc);
            
            coins[ i ]->update( dt );
            
            if (coins[ i ]->deleteReady) {
                coins[ i ]->removeFromParentAndCleanup( true );
                coins[ i ]->destroyPhysics( world );
                coins[ i ] = NULL;
            }
        }
    }
    
    for (int i = kMax_Platfroms-1; i >= 0; i--){
        Sprite *sprite = platforms[i]->sprite;
        
        Vec2 loc = sprite->getPosition();
        
        loc.y-=factor;
        sprite->setPosition(loc);
        
        b2Body *body=platforms[i]->body;
        body->SetTransform(b2Vec2(body->GetPosition().x, body->GetPosition().y- factor*kPhysicsWorldScale()) , 0);
        
    }
    
    //Remove last platform from array
    int i = kMax_Platfroms-1;
    
    if ( platforms[ i ] ) {
        
        Sprite *sprite = platforms[ i ]->sprite;

        if (sprite->getPosition().y + sprite->getContentSize().height * sprite->getScaleY() < 0) {
            platforms[ i ]->destroyPhysics(world);
            platforms[ i ]->removeFromParentAndCleanup( true );
            platforms[ i ] = NULL;
            
            for (int i = kMax_Platfroms-1; i > 0; i--){
                platforms[ i ] = platforms[ i-1 ];
            }
            
            Vec2 loc = platforms[1]->sprite->getPosition();
            Size size = platforms[1]->sprite->getContentSize();

            addPlatform(0, scrollDiff ,Rect(loc.x, loc.y, size.width, size.height));

            
        }
    }
    

}

ax::Rect PZGGamefieldJumpingPlatforms::addPlatform(int pIndex,float dt, ax::Rect rect){
    
    float x = rect.origin.x;
    float y = rect.origin.y;
    float w = rect.size.width;
    float h = rect.size.height;
    
    PZGPlatfrom *platform = NULL;
    
    Sprite *sprite = NULL;
    
    int platform_type_id = rand() % kNum_OFPlatforms;
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
    
    if(platform == NULL){
        return ax::Rect::ZERO;
    }
    
    sprite = platform->sprite;
    
    if (sprite == NULL) {
        return ax::Rect::ZERO;
    }
    
    ax::Size s = kDeviceWinSize();
    
    sprite->setAnchorPoint( ccp(0, 0));
    
    if (isSlowMoveMode) {
        sprite->setPosition( ccp( length, -4) );
    }
    else{
        
        float width = sprite->getContentSize().width;
        
        x+=(width+(CCRANDOM_0_1()*(s.width))) * gameplayInfo->platfromsBetweenDistance*kDeviceScale();
        
        if ((x+width) > s.width) {
            x=fabs(x-(s.width + width));
        }
        
//        printf("x : %f width : %f size %f x %f \n",x,width,s.width,s.height);
        y+=(40 * gameplayInfo->platfromsBetweenDistance*kDeviceScale())+h;
 
        sprite->setPosition(ccp(x,y));
        
        w = sprite->getContentSize().width;
        h = sprite->getContentSize().height;
        
    }
    
    /////////////////////////////////////////////////////////////
    //PHYSICS
    /////////////
    
    Vec2 collisionPosition;
    float width = 0.0f;
    
    if( platform_type_id == 0 ){
        collisionPosition = gameplayInfo->platform1Cp[ 0 ];
        width = gameplayInfo->platform1Cp[ 1 ].x - collisionPosition.x ;
    }
    else if(platform_type_id == 1){
        collisionPosition = gameplayInfo->platform2Cp[ 0 ];
        width = gameplayInfo->platform2Cp[ 1 ].x - collisionPosition.x ;
    }
    else if(platform_type_id == 2){
        collisionPosition = gameplayInfo->platform3Cp[ 0 ];
        width = gameplayInfo->platform3Cp[ 1 ].x - collisionPosition.x ;
    }
    
    platform->collisionPosition = collisionPosition;
    platform->width = width;
    
    platform->initPhysics(world);
    
    platforms[ pIndex ] = platform;
    
    this->addChild(platform);
    
    if (gameplayInfo->randomLevelGeneration == false) {
        addLevel(pIndex, platform_type_id );
    }else {
        
        //UPDATE SPAWN COUNTERS
        if (obstacles_info && gameplayInfo->randomLevelGeneration) {
            for (int i=0; i < obstacles_info->count(); i++) {
                
                PZGArtObstacle *info = (PZGArtObstacle*)obstacles_info->objectAtIndex( i );
                
                //adding dt won't make any difference here, but it is just here for coding reference
                obstacles_spawn_counter[ i ] = dt + CCRANDOM_0_1() * (info->comingRatio*0.5);// + (CCRANDOM_0_1()  * (pIndex/(info->comingRatio*0.5)));
                
                //printf("counter = %f \n",obstacles_spawn_counter[ i ]);
                
                if ((obstacles_spawn_counter[ i ] >= (info->comingRatio*0.5) && obstacles_spawn_counter[ i ] <= info->comingRatio) && info->comingRatio > 0) {
                    addObstacle(pIndex, i );
                }
            }
        }
        
        if (coins_info && gameplayInfo->randomLevelGeneration) {
            for (int i=0; i < coins_info->count(); i++) {
                
                PZGArtCoins *info = (PZGArtCoins*)coins_info->objectAtIndex( i );
                
                //adding dt won't make any difference here, but it is just here for coding reference
                coins_spawn_counter[ i ] = dt + CCRANDOM_0_1() * (info->comingRatio*0.5) + (CCRANDOM_0_1()  * (pIndex/info->comingRatio));
                
                //printf("counter = %f \n",coins_spawn_counter[ i ]);
                
                if ((coins_spawn_counter[ i ] >= (info->comingRatio*0.5) && coins_spawn_counter[ i ] <= info->comingRatio) && info->comingRatio > 0) {
                    addCoins(pIndex, i );
                }
            }
        }
    }
    
    return Rect(x, y, w, h);
    
}

void PZGGamefieldJumpingPlatforms::destoryPhysics(){
    
    for (int i = 0; i < kMax_Platfroms; i++){
        platforms[i]->destroyPhysics(world);
        
        if ( platforms[ i ] ) {
            platforms[ i ]->removeFromParentAndCleanup( true );
            platforms[ i ] = NULL;
        }
        
    }
    
    for (int i = 0; i < kMax_BasicRunnerObsctales; i++) {
        if (obstacles[ i ]) {
            obstacles[ i ]->removeFromParentAndCleanup( true );
            obstacles[ i ]->destroyPhysics( world );
            obstacles[ i ] = NULL;
        }
    }
    
    for (int i = 0; i < kMax_BasicRunnerCoins; i++) {
        if (coins[ i ]) {
            coins[ i ]->removeFromParentAndCleanup( true );
            coins[ i ]->destroyPhysics( world );
            coins[ i ] = NULL;
        }
    }
}

ax::Sprite* PZGGamefieldJumpingPlatforms::getPlatformForIndex(int pIndex) {
    
    PZGPlatfrom* platform = NULL;
    if (pIndex == -1) {
        platform = platforms [kMax_Platfroms-1];
    }else {
        if (pIndex > 0 && pIndex < kMax_Platfroms) {
            platform = platforms [pIndex];
        }
    }
    
    //may be null if index out of range
    return platform->sprite;
}

int PZGGamefieldJumpingPlatforms::getCoinIndex( PZGGameInfoLevelItem *item ){
    for (int i=0; i < coins_info->count(); i++) {
        PZGArtCoins *coin = ( PZGArtCoins* )coins_info->objectAtIndex( i );
        if (coin->index == item->infoObjIndex) {
            return i;
        }
    }
    
    return 0;
}

int PZGGamefieldJumpingPlatforms::getObstacleIndex( PZGGameInfoLevelItem *item ){
    for (int i=0; i < obstacles_info->count(); i++) {
        PZGArtObstacle *obsacle = ( PZGArtObstacle* )obstacles_info->objectAtIndex( i );
        if (obsacle->index == item->infoObjIndex) {
            return i;
        }
    }
    
    return 0;
}

PZGGameInfoLevel* PZGGamefieldJumpingPlatforms::getLevelInfo(float distance, int platform_type_id){
    
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
        
        if( levelInfo->startDistance <= distance && levelInfo->endDistance >= distance){
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

void PZGGamefieldJumpingPlatforms::addLevel(int pIndex, int platform_type_id ){
    
    PZGGameInfoLevel *levelInfo = getLevelInfo(distance, platform_type_id);
    
    if (levelInfo == NULL) {
        return;
    }
    
    Vec2 spritePosition = platforms[ pIndex ]->sprite->getPosition();
    
//    printf("PLATFORM : %d, Location : (%f,%f) \n",pIndex,spritePosition.x,spritePosition.y);
    
    for (int i=0;  i < levelInfo->objects->count(); i++) {
        PZGGameInfoLevelItem *item = (PZGGameInfoLevelItem*)levelInfo->objects->objectAtIndex( i );
        
        if (item->infoObjKey->compare( "Coins" ) == 0) {
            int index = getCoinIndex( item );
            PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( index ), world);
            if (coin == NULL) {
                return;
            }
            
            coin->setPosition( item->position );
            coin->setPosition( ccp(spritePosition.x + coin->getPosition().x * kDeviceScale()*2 - coin->sprite->getContentSize().width/2,
                                   spritePosition.y + coin->getPosition().y * kDeviceScale()*2) );


//            Vec2 loc = ccp(spritePosition.x,spritePosition.y + (coin->sprite->getContentSize().height));
//            coin->setPosition(loc);
            
            coin->setRotation( -item->rotation );
            coin->setScaleX( item->scale.x );
            coin->setScaleY( item->scale.y );
            
            if (coins[ kMax_BasicRunnerCoins-1 ] != NULL) {
                coins[ kMax_BasicRunnerCoins-1 ]->destroyPhysics( world );
                this->removeChild(coins[ kMax_BasicRunnerCoins-1 ], true);
            }
            
            for (int i = kMax_BasicRunnerCoins-1; i > 0; i--){
                coins[ i ] = coins[ i-1 ];
            }
            
            this->addChild( coin, 33 );
            coins[ 0 ] = coin;
        }
        else if( item->infoObjKey->compare( "Obstacles" ) == 0){
            int index = getObstacleIndex( item );
            PZGObstacle *obstacle = PZGObstacle::create( (PZGArtObstacle*)obstacles_info->objectAtIndex( index ), world);
            if (obstacle == NULL) {
                return;
            }
            obstacle->setPosition( item->position );
            obstacle->setPosition( ccp(spritePosition.x + obstacle->getPosition().x * kDeviceScale()*2,
                                       spritePosition.y + obstacle->getPosition().y * kDeviceScale()*2) );
            
//            Vec2 loc = ccp(spritePosition.x,spritePosition.y + (obstacle->sprite->getContentSize().height));
//            obstacle->setPosition(loc);
            
            obstacle->setRotation( -item->rotation );
            obstacle->setScaleX( item->scale.x );
            obstacle->setScaleY( item->scale.y );
            //obstacle->update(0);
            
            if (obstacles[ kMax_BasicRunnerObsctales-1 ] != NULL) {
                obstacles[ kMax_BasicRunnerObsctales-1 ]->destroyPhysics( world );
                this->removeChild(obstacles[ kMax_BasicRunnerObsctales-1 ], true);
            }
            for (int i = kMax_BasicRunnerObsctales-1; i > 0; i--){
                obstacles[ i ] = obstacles[ i-1 ];
            }
            
            this->addChild( obstacle,  32 - (obstacles_info->count() - 0));
            obstacles[ 0 ] = obstacle;
        }
    }
}

void PZGGamefieldJumpingPlatforms::addCoins(int pIndex, int typeId ){
    
    Size screen = Director::getInstance()->getWinSize();
    
    Vec2 spritePosition = platforms[ pIndex ]->sprite->getPosition();
    Vec2 collisionPosition = platforms[ pIndex ]->collisionPosition;
    float collisionWidth = platforms[ pIndex ]->width;
    
    if (coins_info) {
        
        
        PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( typeId ), world);
        if (coin == NULL) {
            
            coin->removeFromParentAndCleanup( true );
            coin->destroyPhysics( world );
            coin = NULL;
            
            return;
        }
        
        Vec2 loc = ccp(spritePosition.x + collisionPosition.x*kDeviceScale() + CCRANDOM_0_1()*collisionWidth*kDeviceScale(),
                          spritePosition.y + collisionPosition.y*kDeviceScale() + coin->sprite->getContentSize().height);
        
        coin->setPosition(loc);

        coin->update(0);
        
        if ((coin->getPosition().x+coin->sprite->getContentSize().width) > screen.width) {
            
            coin->removeFromParentAndCleanup( true );
            coin->destroyPhysics( world );
            coin = NULL;
            
            printf("coin\n");
            return;
        }
        
        Rect rn = RectMake(coin->getPosition().x - coin->sprite->getContentSize().width * 0.5,
                               coin->getPosition().y - coin->sprite->getContentSize().height * 0.5,
                               coin->sprite->getContentSize().width,
                               coin->sprite->getContentSize().height);
        
        //check for obstacle overlap
        for (int i=0; i < kMax_BasicRunnerObsctales; i++) {
            if (obstacles[ i ]) {
                Rect r = RectMake(obstacles[ i ]->getPosition().x - obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->getPosition().y - obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale,
                                      obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale);
                
                if (r.intersectsRect( rn )) {
                    
                    coin->removeFromParentAndCleanup( true );
                    coin->destroyPhysics( world );
                    coin = NULL;
                    
                    return;
                }
            }
        }
        
        // check for overlap with coins
        for (int i=0; i < kMax_BasicRunnerCoins; i++) {
            if (coins[ i ]) {
                Rect r = RectMake(coins[ i ]->getPosition().x - coins[ i ]->sprite->getContentSize().width * 0.5,
                                      coins[ i ]->getPosition().y - coins[ i ]->sprite->getContentSize().height * 0.5,
                                      coins[ i ]->sprite->getContentSize().width,
                                      coins[ i ]->sprite->getContentSize().height);
                if (r.intersectsRect( rn )) {
                    
                    coin->removeFromParentAndCleanup( true );
                    coin->destroyPhysics( world );
                    coin = NULL;
                    
                    return;
                }
            }
        }
        
        if (coins[ kMax_BasicRunnerCoins-1 ] != NULL) {
            coins[ kMax_BasicRunnerCoins-1 ]->destroyPhysics( world );
            this->removeChild(coins[ kMax_BasicRunnerCoins-1 ], true);
        }
        
        for (int i = kMax_BasicRunnerCoins-1; i > 0; i--){
            coins[ i ] = coins[ i-1 ];
        }
        
        this->addChild( coin, 33 );
        coins[ 0 ] = coin;
    }
}
void PZGGamefieldJumpingPlatforms::addObstacle(int pIndex, int typeId ){
    
    Size screen = Director::getInstance()->getWinSize();
    
    Vec2 spritePosition = platforms[ pIndex ]->sprite->getPosition();
    Vec2 collisionPosition = platforms[ pIndex ]->collisionPosition;
    float collisionWidth = platforms[ pIndex ]->width;
    
    if (obstacles_info) {
        
        PZGArtObstacle *obstInfo = (PZGArtObstacle*)obstacles_info->objectAtIndex( typeId );
        PZGObstacle *obstacle = PZGObstacle::create( obstInfo, world);
        
        if (obstacle == NULL) {
            return;
        }
        
       
        Vec2 loc = ccp(spritePosition.x + collisionPosition.x*kDeviceScale() + CCRANDOM_0_1()*collisionWidth*kDeviceScale() ,
                          spritePosition.y + collisionPosition.y*kDeviceScale() + obstacle->sprite->getContentSize().height * obstacle->obstacle_scale);
        
        obstacle->setPosition(loc);
        obstacle->update(0);
        
        obstacle->velocityLinear = ccp(0,0);
        
        if ((obstacle->getPosition().x + obstacle->sprite->getContentSize().width) > screen.width) {
            
            obstacle->removeFromParentAndCleanup( true );
            obstacle->destroyPhysics( world );
            obstacle = NULL;
            
            return;
        }
        
        Rect rn = RectMake(obstacle->getPosition().x - obstacle->sprite->getContentSize().width * obstacle->obstacle_scale * 0.5 * 2,
                               obstacle->getPosition().y - obstacle->sprite->getContentSize().height * obstacle->obstacle_scale * 0.5,
                               obstacle->sprite->getContentSize().width * obstacle->obstacle_scale * 2,
                               obstacle->sprite->getContentSize().height * obstacle->obstacle_scale);
        
        //check for obstacle overlap
        for (int i=0; i < kMax_BasicRunnerObsctales; i++) {
            if (obstacles[ i ]) {
                Rect r = RectMake(obstacles[ i ]->getPosition().x - obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->getPosition().y - obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale,
                                      obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale);
                
                if (r.intersectsRect( rn )) {
                    
                    obstacle->removeFromParentAndCleanup( true );
                    obstacle->destroyPhysics( world );
                    obstacle = NULL;
                    
                    return;
                }
            }
        }
        
        // check for overlap with coins
        for (int i=0; i < kMax_BasicRunnerCoins; i++) {
            if (coins[ i ]) {
                Rect r = RectMake(coins[ i ]->getPosition().x - coins[ i ]->sprite->getContentSize().width * 0.5,
                                      coins[ i ]->getPosition().y - coins[ i ]->sprite->getContentSize().height * 0.5,
                                      coins[ i ]->sprite->getContentSize().width,
                                      coins[ i ]->sprite->getContentSize().height);
                if (r.intersectsRect( rn )) {
                    
                    obstacle->removeFromParentAndCleanup( true );
                    obstacle->destroyPhysics( world );
                    obstacle = NULL;
                    
                    return;
                }
            }
        }
        
        if (obstacles[ kMax_BasicRunnerObsctales-1 ] != NULL) {
            obstacles[ kMax_BasicRunnerObsctales-1 ]->destroyPhysics( world );
            this->removeChild(obstacles[ kMax_BasicRunnerObsctales-1 ], true);
        }
        for (int i = kMax_BasicRunnerObsctales-1; i > 0; i--){
            obstacles[ i ] = obstacles[ i-1 ];
        }
        
        this->addChild( obstacle,  32 - (obstacles_info->count() - typeId));
        obstacles[ 0 ] = obstacle;
    }
}

void PZGGamefieldJumpingPlatforms::update(float dt){
    
    //UPDATE OBJECT COUNTERS
    for (int i = 0; i < kMax_BasicRunnerObsctales; i++) {
        if (obstacles[ i ]) {
            obstacles[ i ]->update( dt );
            
            if (obstacles[ i ]->deleteReady) {
                obstacles[ i ]->removeFromParentAndCleanup( true );
                obstacles[ i ]->destroyPhysics( world );
                obstacles[ i ] = NULL;
            }
        }
    }
    
    
    for (int i = 0; i < kMax_BasicRunnerCoins; i++) {
        if (coins[ i ]) {
            coins[ i ]->update( dt );
            
            if (coins[ i ]->deleteReady) {
                coins[ i ]->removeFromParentAndCleanup( true );
                coins[ i ]->destroyPhysics( world );
                coins[ i ] = NULL;
            }
        }
    }
    
    addPlatforms(dt);
}
