//
//  PZGGamefieldPlatforms.cpp
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#include "PZGGamefieldPlatforms.h"
#include "constants.h"

USING_NS_AX;

Scene* PZGGamefieldPlatforms::scene(){
    
	Scene *scene = Scene::create();
	PZGGamefieldPlatforms *layer = PZGGamefieldPlatforms::create();
    
	scene->addChild(layer);
	return scene;
}

bool PZGGamefieldPlatforms::init()
{
	if ( !Layer::init() )	{
		return false;
	}
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey( "GameplaySettings" );
    gameplayInfo = (PZGGameplayBasicRunner*)array->objectAtIndex( 0 );
    
    obstacles_info = (__Array*)gsd->artResource->objectForKey( "Obstacles" );
    coins_info = (__Array*)gsd->artResource->objectForKey( "Coins" );
    
    __Array* levels = (__Array*)gsd->gameInfoResource->objectForKey( "Levels" );

    if (levels) {
        for (int p = 0 ; p < kNumOFPlatforms; p++) {
            levels_info[ p ] = __Array::create();
            levels_info[ p ]->retain();
            
            //filtering all levels and adding only levels that ralated to curent platfroms
            for (int i=0; i < levels->count(); i++) {
                PZGGameInfoLevel *level = (PZGGameInfoLevel*)levels->objectAtIndex( i );
                if (
                    level->platformInfoKey->compare( gameplayInfo->platform1Art->key->getCString() ) == 0 &&
                    level->platformInfoIndex == p
                    ) {
                    levels_info[ p ]->addObject( level );
                }
            }
            
            printf(" --- platform_type_id: %d  COUNT: %d\n", p, levels_info[ p ]->count());
        }        
    }
    
    
    reset();

    return true;
}

void PZGGamefieldPlatforms::reset(){
    length = 0;
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
    
    Vec2 spritePosition = platforms[ 0 ]->sprite->getPosition();
    
    for (int i=0;  i < levelInfo->objects->count(); i++) {
        PZGGameInfoLevelItem *item = (PZGGameInfoLevelItem*)levelInfo->objects->objectAtIndex( i );
        
        if (item->infoObjKey->compare( "Coins" ) == 0) {
            int index = getCoinIndex( item );
            PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( index ), world);
            if (coin == NULL) {
                return;
            }
            
            coin->setPosition( item->position );
            coin->setPosition( ccp(spritePosition.x + coin->getPosition().x * kDeviceScale()*2,
                                   spritePosition.y + coin->getPosition().y * kDeviceScale()*2) );
            coin->setRotation( -item->rotation );
            coin->setScaleX( item->scale.x );
            coin->setScaleY( item->scale.y );
            
            if (coins[ kMaxBasicRunnerCoins-1 ] != NULL) {
                coins[ kMaxBasicRunnerCoins-1 ]->destroyPhysics( world );
                this->removeChild(coins[ kMaxBasicRunnerCoins-1 ], true);
            }
            
            for (int i = kMaxBasicRunnerCoins-1; i > 0; i--){
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
            obstacle->setRotation( -item->rotation );
            obstacle->setScaleX( item->scale.x );
            obstacle->setScaleY( item->scale.y );
            
            if (obstacles[ kMaxBasicRunnerObsctales-1 ] != NULL) {
                obstacles[ kMaxBasicRunnerObsctales-1 ]->destroyPhysics( world );
                this->removeChild(obstacles[ kMaxBasicRunnerObsctales-1 ], true);
            }
            for (int i = kMaxBasicRunnerObsctales-1; i > 0; i--){
                obstacles[ i ] = obstacles[ i-1 ];
            }
            
            this->addChild( obstacle,  32 - (obstacles_info->count() - 0));
            obstacles[ 0 ] = obstacle;
        }
    }
}

void PZGGamefieldPlatforms::addCoins( int typeId ){
    
    if (length < 1200*kDeviceScale()){
        return;
    }
    
    Size screen = Director::getInstance()->getWinSize();
    Vec2 spritePosition = platforms[ 0 ]->sprite->getPosition();
    Vec2 collisionPosition = platforms[ 0 ]->collisionPosition;// platfroms_collision[ 0 ];
    float collisionWidth = platforms[ 0 ]->width;// platforms_width[ 0 ];
    
    if (coins_info) {

        
        PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( typeId ), world);
        if (coin == NULL) {
            return;
        }
        coin->setPosition( ccp(spritePosition.x + collisionPosition.x*kDeviceScale() + CCRANDOM_MINUS1_1()*collisionWidth*kDeviceScale(),
                               spritePosition.y + collisionPosition.y*kDeviceScale() + coin->sprite->getContentSize().height*0.5) );
                
        if (coin->getPosition().x < screen.width*1.1) {
            printf("coin\n");
            return;
        }
        
        //check for obstacle overlap
        for (int i=0; i < kMaxBasicRunnerObsctales; i++) {
            if (obstacles[ i ]) {
                Rect r = RectMake(obstacles[ i ]->getPosition().x - obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->getPosition().y - obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale * 0.5,
                                      obstacles[ i ]->sprite->getContentSize().width * obstacles[ i ]->obstacle_scale,
                                      obstacles[ i ]->sprite->getContentSize().height * obstacles[ i ]->obstacle_scale);
                
                Rect rn = RectMake(coin->getPosition().x - coin->sprite->getContentSize().width * 0.5,
                                       coin->getPosition().y - coin->sprite->getContentSize().height * 0.5,
                                       coin->sprite->getContentSize().width,
                                       coin->sprite->getContentSize().height);
                
                if (r.intersectsRect( rn )) {
                    return;
                }
            }
        }
        
        
        if (coins[ kMaxBasicRunnerCoins-1 ] != NULL) {
            coins[ kMaxBasicRunnerCoins-1 ]->destroyPhysics( world );
            this->removeChild(coins[ kMaxBasicRunnerCoins-1 ], true);
        }
        
        for (int i = kMaxBasicRunnerCoins-1; i > 0; i--){
            coins[ i ] = coins[ i-1 ];
        }
        
        this->addChild( coin, 33 );
        coins[ 0 ] = coin;
    }
}
void PZGGamefieldPlatforms::addObstacle( int typeId ){
    
    if (length < 1200*kDeviceScale()){
        return;
    }
    
    Size screen = Director::getInstance()->getWinSize();
    
    Vec2 spritePosition = platforms[ 0 ]->sprite->getPosition();
    Vec2 collisionPosition = platforms[ 0 ]->collisionPosition;// platfroms_collision[ 0 ];
    float collisionWidth = platforms[ 0 ]->width;// platforms_width[ 0 ];
    
    if (obstacles_info) {
        
        PZGArtObstacle *obstInfo = (PZGArtObstacle*)obstacles_info->objectAtIndex( typeId );
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
        
        if (obstacles[ kMaxBasicRunnerObsctales-1 ] != NULL) {
            obstacles[ kMaxBasicRunnerObsctales-1 ]->destroyPhysics( world );
            this->removeChild(obstacles[ kMaxBasicRunnerObsctales-1 ], true);
        }
        for (int i = kMaxBasicRunnerObsctales-1; i > 0; i--){
            obstacles[ i ] = obstacles[ i-1 ];
        }
        
        this->addChild( obstacle,  32 - (obstacles_info->count() - typeId));
        obstacles[ 0 ] = obstacle;
    }
}

void PZGGamefieldPlatforms::update(float dt){
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
        for (int i=0; i < coins_info->count(); i++) {
            PZGArtCoins *info = (PZGArtCoins*)coins_info->objectAtIndex( i );
            coins_spawn_counter[ i ] += dt;
            
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
            
            if (obstacles[ i ]->deleteReady) {
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
            
            if (coins[ i ]->deleteReady) {
                coins[ i ]->removeFromParentAndCleanup( true );
                coins[ i ]->destroyPhysics( world );
                coins[ i ] = NULL;
            }
        }
    }

    length -= s;
    
    if (length < 1200*kDeviceScale()) {
        addPlatform();
    }

}


