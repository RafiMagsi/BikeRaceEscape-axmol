//
//  PZGGameFieldScene.cpp
//  PZPlayer
//
//  Created by Nik Rudenko on 6/19/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#include "PZGGameFieldScene.h"
#include "PZGBackgroundScene.h"

#include "PZGGameOverScene.h"
#include "PZGPauseMenuScene.h"

#include "PZGContactListener.h"

#include "PZLegacyCompat.h"
#include "constants.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    #include "Neocortex.h"
#endif

USING_NS_AX;

static PZGGameFieldScene *gfScene;

Scene* PZGGameFieldScene::scene(){
    
	Scene *scene = Scene::create();
	PZGGameFieldScene *layer = PZGGameFieldScene::create();
    gfScene = layer;
    
	scene->addChild(layer);
	return scene;
}

PZGGameFieldScene* PZGGameFieldScene::shared(){
    return gfScene;
}

void PZGGameFieldScene::reset(){
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    if (!gsd) {
        AXLOGE("PZGGameFieldScene::reset: gsd is null");
        return;
    }

    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GameplaySettings");
    if (!array || array->count() == 0) {
        AXLOGE("PZGGameFieldScene::reset: GameplaySettings missing/empty");
        return;
    }
    PZGGameInfoObject* gameInfoObject = dynamic_cast<PZGGameInfoObject*>(array->objectAtIndex(0));
    if (!gameInfoObject) {
        AXLOGE("PZGGameFieldScene::reset: GameplaySettings[0] is null/wrong type");
        return;
    }
        
    gf_speed_max = gameInfoObject->gameSpeedMax;
    gf_speed_increaser =  gameInfoObject->gameSpeedIncreaser;
    gf_speed = gameInfoObject->gameSpeedMin;
    
    gf_distance = 0.0;

    if (character) {
        character->reset();
    }

    
    PZSettingsController* sc = PZSettingsController::shared();
    gf_coins = sc->coins;
    
    
    for (int i=0; i < kEnemyMax; i++) {
        PZGObstacle *en = obstacles[ i ];
        if (en){
            en->destroyPhysics( world );
            this->removeChild( en, true );
        }
        obstacles[ i ] = NULL;
    }
    for (int i=0; i < kCoinMax; i++) {
        PZGCoin *c = coins[ i ];
        if (c){
            c->destroyPhysics( world );
            this->removeChild( c, true );
        }
        coins[ i ] = NULL;
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
            AXLOGW("PZGGameFieldScene::reset: missing gameStartSound at index={} (sounds count={})",
                   kSoundIDGameStart, sounds->count());
        }
    }
    
    for (int i=0; i < parallax_n; i++) {
        if (parallax[ i ]) {
            parallax[ i ]->reset();
        }
    }
}


bool PZGGameFieldScene::init()
{
	if ( !Layer::init() )	{
		return false;
	}

    AXLOGI("PZGGameFieldScene::init begin");
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    this->setKeypadEnabled(true);
#endif
    
    restrictObstacles = false;
    restrictCoins = false;
    preventObstaclesOverlappingObstackes = false;
    preventObstaclesOverlappingCoins = false;
    
	Size screen = Director::getInstance()->getWinSize();
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    __Array* array = (__Array*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    if (!array || array->count() == 0) {
        AXLOGE("PZGGameFieldScene::init: GeneralApplicationSettings missing/empty");
        return false;
    }
    PZGGameInfoGeneral * gameInfoGeneral = (PZGGameInfoGeneral*)array->objectAtIndex(0);
    if (!gameInfoGeneral) {
        AXLOGE("PZGGameFieldScene::init: GeneralApplicationSettings[0] is null");
        return false;
    }
    isPortraitMode = gameInfoGeneral->isPortraitMode;
    

    backgroundLayers = (__Array*)gsd->artResource->objectForKey( "BackgroundLayers" );
    if ( backgroundLayers ){
        parallax_n =  backgroundLayers->count();
    }
    else{
        parallax_n = 0;
    }
    bool isCharacterAdded = false;
    
    for (int i=0; i < parallax_n; i++) {
        PZGArtBackground *bgLayer = (PZGArtBackground*)backgroundLayers->objectAtIndex( i );
        if ( bgLayer->playGroundLayer == false ) {
            Sprite *bgSprite = bgLayer->getResource();
            if (bgSprite) {
                
                float totalSize = 0;
                float targetSize;
                if (isPortraitMode) {
                    targetSize = screen.height * 2.2;
                }
                else{
                    targetSize =screen.width * 2.2;
                }
                __Array *bgSpriteArray = __Array::create();
                
                Size size = Director::getInstance()->getWinSizeInPixels();
                Sprite *sprt;
                int j=0;
                
                while (totalSize <= targetSize || j==1) {
                    sprt = bgLayer->getResource();
                    sprt->setScale( bgLayer->scale );
                    sprt->setOpacity( bgLayer->opacity*255 );
                    
                    bgSpriteArray->addObject( sprt );
                    
                    if (isPortraitMode) {
                        totalSize += sprt->getContentSize().height * sprt->getScale();
                    }
                    else{
                        totalSize += sprt->getContentSize().width * sprt->getScale();
                    }
                    j+=1;
                }
                
                
                parallax[ i ] = CCParallaxScrollNode::create();
                parallax[ i ]->setAnchorPoint( ccp(0.5, 0.5) );
                if (isPortraitMode) {
                    parallax[ i ]->addInfiniteScrollWithObjects(bgSpriteArray,
                                                                0,
                                                                ccp(0.0, bgLayer->speed),
                                                                ccp(bgLayer->position*screen.width - bgSprite->getContentSize().width*0.5, 0),
                                                                ccp(0,1)
                                                                );
                }
                else{
                    parallax[ i ]->addInfiniteScrollWithObjects(bgSpriteArray,
                                                                0,
                                                                ccp(bgLayer->speed, 0.0),
                                                                ccp(0, bgLayer->position*screen.height - bgSprite->getContentSize().height*0.5),
                                                                ccp(1,0)
                                                                );
                    float ratio = Director::getInstance()->getWinSize().height / (640*kDeviceScale());
//                    printf("%f / %f", Director::getInstance()->getWinSize().height, 640*kDeviceScale());
                    parallax[ i ]->setScale( ratio );
                }

                this->addChild( parallax[ i ], i*10);

            }
            else{
                parallax[ i ] = NULL;
            }
        
        }
        else{
            PZGArtBackgroundPlay *playLayer = dynamic_cast<PZGArtBackgroundPlay*>(bgLayer);
            if (playLayer) {
                boundLeft  = playLayer->boundLeft;
                boundUp  = playLayer->boundUp;
                boundRight  = playLayer->boundRight;
                boundDown  = playLayer->boundDown;
            }
            else{
                boundLeft = 0;
                boundUp = 0;
                boundRight = 0;
                boundDown = 0;
            }
            
            
            Size screen = Director::getInstance()->getWinSize();
            
            playArea = RectMake(boundLeft * screen.width,
                                  boundUp * screen.height,
                                  screen.width - boundRight * screen.width ,
                                  screen.height - boundDown * screen.height);
            
            if (!isCharacterAdded) {
                isCharacterAdded = true;
                character_depth_z = i*10 + 5;
                if (character) {
                    this->addChild(character, i*10 + 5);
                }
            }
        }
    }    

    
    if (!isCharacterAdded) {
        isCharacterAdded = true;
        character_depth_z = parallax_n*10 + 5;
        this->addChild(character, parallax_n*10+5);
    }
    
    
    reset();
    
  
    __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
    if (sounds && sounds->count() > kSoundIDCoinPickUp) {
        coin_pickup_sound = dynamic_cast<PZGSoundData*>(sounds->objectAtIndex(kSoundIDCoinPickUp));
        if (coin_pickup_sound) {
            coin_pickup_sound->preloadSound();
        }
    } else {
        coin_pickup_sound = nullptr;
    }
    
    PZSettingsController* sc = PZSettingsController::shared( );
    gf_coins = sc->coins;

    this->schedule( AX_SCHEDULE_SELECTOR( PZGGameFieldScene::update ) );

    
//    if (gameInfoGeneral->isDebugMode) {
//        setDebugModeEnabled(true);
//    }
    
    AXLOGI("PZGGameFieldScene::init end");
    return true;
}

void PZGGameFieldScene::load(const char* keyName){
    PZGBaseMenuScene::load( keyName );
    
    tip_counter = 0;
    
    PZSettingsController* sc = PZSettingsController::shared();
    
    PZGSharedData *sd = PZGSharedData::sharedInstanse();
    PZGGameInfoIAP *iapInfo = nullptr;
    if (sd) {
        __Array *array = (__Array*) sd->gameInfoResource->objectForKey("IAPSettings");
        if (array && array->count() > 0) {
            iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
        }
    }
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        pauseButton = (MenuItemSprite*)menu->getChildByTag( kUIIDGameFieldPauseButton ); //INDEX 0 is for PAUSE BUTTON
        if (pauseButton) {
            pauseButton->setCallback(AX_CALLBACK_1(PZGGameFieldScene::pauseMenuCallback, this));
        }
        
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 8 );
        if (item) {
            if (!iapInfo || sc->removeAds || iapInfo->removeAd_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::menuUpgradeCallback, this));
            }
        }
        
        item = (MenuItemSprite*)menu->getChildByTag( 7 );
        if (item) {
            if (!iapInfo || iapInfo->coinShop_enabled == false) {
                item->setVisible( false );
            }
            else{
                item->setVisible( true );
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::coinShopCallback, this));
            }
        }
        item = (MenuItemSprite*)menu->getChildByTag( 6 );
        if (item) {
            if (iapInfo && iapInfo->kidMode_enabled && sc->kidMode == false) {
                item->setVisible(true);
                item->setCallback(AX_CALLBACK_1(PZGBaseMenuScene::buyKidModeCallback, this));
            }
            else{
                item->setVisible(false);
            }
        }
    }
    Sprite *sprite = (Sprite*)this->getChildByTag( 4 ); //INDEX 4 is for TIP#1
    if (sprite) {
        tip1 = sprite;
    }
    
    sprite = (Sprite*)this->getChildByTag( 5 ); //INDEX 5 is for TIP#2
    if (sprite) {
        tip2 = sprite;
    }

    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    
    // ADDING LABELS
    __Array* uiItems = (__Array*)gsd->artResource->objectForKey( "InterfaceGF" );
    if (!uiItems || uiItems->count() <= 3) {
        AXLOGE("PZGGameFieldScene::load: InterfaceGF missing/too small (count={})", uiItems ? uiItems->count() : 0);
        return;
    }

    PZGArtInterface* coinLabelPosition = (PZGArtInterface*)uiItems->objectAtIndex(3);
    PZGArtInterface* distamceLabelPosition = (PZGArtInterface*)uiItems->objectAtIndex(2);
    if (!coinLabelPosition || !distamceLabelPosition) {
        AXLOGE("PZGGameFieldScene::load: InterfaceGF label positions are null (coin={}, dist={})",
               (void*)coinLabelPosition, (void*)distamceLabelPosition);
        return;
    }
    
    Size size = Director::getInstance()->getWinSize();
    
    distance_label = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
    distance_label->setScaleX( distamceLabelPosition->scale_x);
    distance_label->setScaleY( distamceLabelPosition->scale_y);
    distance_label->setPosition( distamceLabelPosition->getPosition() );
    distance_label->setRotation( -distamceLabelPosition->rotation );
    distance_label->setAlignment( TextHAlignment::RIGHT );
    distance_label->setAnchorPoint( ccp(0, 0.5) );    
    this->addChild( distance_label, 300 );

    coins_label = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
    coins_label->setScaleX( coinLabelPosition->scale_x);
    coins_label->setScaleY( coinLabelPosition->scale_y);
    coins_label->setAlignment( TextHAlignment::RIGHT );
    coins_label->setAnchorPoint( ccp(0, 0.5) );
    coins_label->setPosition( coinLabelPosition->getPosition() );
    coins_label->setRotation( -coinLabelPosition->rotation );
    this->addChild( coins_label, 300 );
        
    coins_add_label = Label::createWithBMFont(gsd->getFullPath("MainFont.fnt")->getCString(), "0");
    coins_add_label->setPosition( ccp(-20,-20) );
    coins_add_label->setScale(0.8);
    coins_add_label->setOpacity( 0 );
    this->addChild( coins_add_label, 200 );
    
    char c[ 16 ];
    sprintf( c, "%d", (int)gf_coins);
    coins_label->setString( c );

    gfScene = this;
    
#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
    PZ::Neocortex::shared()->onPlaying();
#endif
    
}

void PZGGameFieldScene::initPhysics( Vec2 gravity ){

    world = new b2World( b2Vec2(gravity.x, gravity.y) );
    
    PZGContactListener *contactListener = new PZGContactListener();
    world->SetContactListener(contactListener);
    
    GLESDebugDraw* debugdraw = new GLESDebugDraw(1.0f);
    Settings *settings = new Settings();
    uint32 flags = 0;
    flags += settings->drawShapes			* b2Draw::e_shapeBit;
    flags += settings->drawJoints			* b2Draw::e_jointBit;
    flags += settings->drawAABBs			* b2Draw::e_aabbBit;
    flags += settings->drawPairs			* b2Draw::e_pairBit;
    flags += settings->drawCOMs				* b2Draw::e_centerOfMassBit;
    debugdraw->SetFlags(flags);
    world->SetDebugDraw((b2Draw*)debugdraw);
}

void PZGGameFieldScene::setDebugModeEnabled(bool enabled){

    if (enabled) {
        Scene *scene = GLESDEbugDrawLayer::scene();
        scene->setTag( kDebugSceneTag );
        this->addChild(scene, 100);
        GLESDEbugDrawLayer* debugLayer = (GLESDEbugDrawLayer*)scene->getChildren().at(0);
        debugLayer->world = world;
        debugLayer->playArea = playArea;
    }
    else{
        this->removeChildByTag(kDebugSceneTag, true);
    }
    
}

void PZGGameFieldScene::removePauseMenu(){
    gamePause = false;

    pauseButton->setVisible( true );
    this->removeChildByTag(20, true);
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 8 );
        if (item) {
            item->setVisible( true );
        }
        item = (MenuItemSprite*)menu->getChildByTag( 7 );
        if (item) {
            item->setVisible( true );
        }
        item = (MenuItemSprite*)menu->getChildByTag( 6 );
        if (item) {
            item->setVisible( true );
        }
    }

}

void PZGGameFieldScene::pauseMenuCallback( Object* pSender ){
    if (!gamePause) {
        pauseButton->setVisible( false );
        
        gamePause = true;
        
        CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();

        // Overlay pause menu as a layer (avoid nesting a Scene as a child node).
        auto* pauseMenuScene = PZGPauseMenuScene::create();
        if (!pauseMenuScene) {
            AXLOGE("pauseMenuCallback: failed to create PZGPauseMenuScene layer (init returned false)");
            gamePause = false;
            pauseButton->setVisible(true);
            return;
        }
        pauseMenuScene->setTag(20);
        pauseMenuScene->baseScene = this;
        pauseMenuScene->load("InterfacePM");
        this->addChild(pauseMenuScene, 200);

        
        PZGSharedData *gsd = PZGSharedData::sharedInstanse();
        __Array* sounds = (__Array*)gsd->soundResource->objectForKey("Sounds");
        if (sounds && sounds->count() > kSoundIDButtonPressed) {
            auto* buttonClickedSound = dynamic_cast<PZGSoundData*>(sounds->objectAtIndex(kSoundIDButtonPressed));
            if (buttonClickedSound) {
                buttonClickedSound->playAsSound(false);
            }
        }
        
        Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
        if (menu) {
            MenuItemSprite *item;
            item = (MenuItemSprite*)menu->getChildByTag( 8 );
            if (item) {
                item->setVisible( false );
            }
            item = (MenuItemSprite*)menu->getChildByTag( 7 );
            if (item) {
                item->setVisible( false );
            }
            item = (MenuItemSprite*)menu->getChildByTag( 6 );
            if (item) {
                item->setVisible(false);
            }
        }
        
        if (character) {
            character->shooting = false;
            if (character->shootingSound){
                character->shootingSound->stopSound();
            }
        }
    }
}

void PZGGameFieldScene::gameRestart(){
    
    reset();
    
    gamePause = false;
    gf_distance = 0;
    this->gameOverCallBack( false );
}

void PZGGameFieldScene::coinPickup( PZGCoin* _coin ){

    if (_coin->disabled == true){
        return;
    }
    
    if (coin_pickup_sound) {
        coin_pickup_sound->playAsSound( false );
    }
    
    _coin->disabled = true;
    _coin->deleteReady = true;
    
    Size size = Director::getInstance()->getWinSize();
  
    gf_coins += _coin->reward;
    
    PZSettingsController* sc = PZSettingsController::shared();
    sc->coins += _coin->reward;
    
    char c[16];
    
    sprintf( c, "%d", (int)gf_coins);
    coins_label->setString( c );


    coins_add_label->setPosition( _coin->getPosition() );
    sprintf( c, "%d", (int)_coin->reward );
    coins_add_label->setString( c );
    coins_add_label->setOpacity(255);
    
    coins_add_counter = 1;
    coins_add_velocity = 1;
    
//#if (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
//    CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect( coins_picup_sound );
//#endif
    
}

void PZGGameFieldScene::gameOverCallBack(bool _isGameOver){
    gameOver = _isGameOver;
    
    MenuItemSprite *pauseButton = NULL;
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        pauseButton = (MenuItemSprite*)menu->getChildByTag( kUIIDGameFieldPauseButton ); //INDEX 0 is for PAUSE BUTTON
    }
    
    if (_isGameOver) {
        if (pauseButton) {
            pauseButton->setEnabled(false);
        }
        
        PZSettingsController* sc = PZSettingsController::shared();
        sc->save();
        
        if (character) {
            character->dead();
            character->shooting = false;
            if (character->shootingSound){
                character->shootingSound->stopSound();
            }
        } else {
            AXLOGW("gameOverCallBack: character is null");
        }
        
        // Overlay game-over as a layer (avoid nesting a Scene as a child node).
        auto* gameOverScene = PZGGameOverScene::create();
        if (!gameOverScene) {
            AXLOGE("gameOverCallBack: failed to create PZGGameOverScene layer (init returned false)");
            return;
        }
        gameOverScene->setTag(10);
        this->addChild(gameOverScene, 200);
        gameOverScene->baseScene = this;
        gameOverScene->load("InterfaceGO");
        gameOverScene->setDistance(gf_distance);
        
        Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
        if (menu) {
            MenuItemSprite *item;
            item = (MenuItemSprite*)menu->getChildByTag( 8 );
            if (item) {
                item->setVisible( false );
            }
            item = (MenuItemSprite*)menu->getChildByTag( 7 );
            if (item) {
                item->setVisible( false );
            }
            item = (MenuItemSprite*)menu->getChildByTag( 6 );
            if (item) {
                item->setVisible(false);
            }
        }
        
        if (gameStartSound) {
            gameStartSound->stopSound();
        }

    }
    else {
        
        PZSettingsController* sc = PZSettingsController::shared();
        
        PZGSharedData *sd = PZGSharedData::sharedInstanse();
        PZGGameInfoIAP *iapInfo = nullptr;
        if (sd) {
            if (auto* array = (__Array*) sd->gameInfoResource->objectForKey("IAPSettings"); array && array->count() > 0) {
                iapInfo = (PZGGameInfoIAP*)array->objectAtIndex(0);
            }
        }
        
        this->removeChildByTag(10, true);
        
        if (pauseButton) {
            pauseButton->setEnabled(true);
        }
        
        Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
        if (menu) {
            MenuItemSprite *item;
            item = (MenuItemSprite*)menu->getChildByTag( 8 );
            if (item) {
                if (iapInfo && sc->removeAds == false && iapInfo->removeAd_enabled == true) {
                    item->setVisible( true );
                }
            }
            item = (MenuItemSprite*)menu->getChildByTag( 7 );
            if (item) {
                if (iapInfo && iapInfo->coinShop_enabled == true){
                    item->setVisible( true );
                }
            }
            item = (MenuItemSprite*)menu->getChildByTag( 6 );
            if (item) {
                if (iapInfo && iapInfo->kidMode_enabled && sc->kidMode == false){
                    item->setVisible( true );
                }
            }
        }
    }
}


void PZGGameFieldScene::addObstacle( int typeId ){
    if (obstacles_info == NULL || obstacles_info->count() == 0) {
        return;
    }
    
    PZGArtObstacle *obstInfo = (PZGArtObstacle*)obstacles_info->objectAtIndex( typeId );
    
    PZGObstacle *obstacle = PZGObstacle::create( obstInfo, world);
    if (obstacle == NULL){
        return;
    }
    
    if (restrictObstacles) {
        
        Size screen = Director::getInstance()->getWinSize();
        
        Vec2 point = ccp(screen.width * boundLeft,screen.height * boundUp);
        
        screen.width = screen.width - ((screen.width * boundRight)+point.x);
        screen.height = screen.height - ((screen.height * boundDown)+point.y);
        
        Size spriteSize = obstacle->sprite->getContentSize();
        
        screen.width-=spriteSize.width+boundRight*kDeviceScale();
        screen.height-=spriteSize.height+boundDown*kDeviceScale();
        
        if (point.x > 0) {
            point.x+=boundLeft*kDeviceScale();
        }
        
        if (point.y > 0) {
            point.y+=boundUp*kDeviceScale();
        }
        
        if (isPortraitMode) {
            obstacle->setPosition( ccp(point.x + (CCRANDOM_0_1() * screen.width) , point.y + screen.height*1.1) );
        }else{
            obstacle->setPosition( ccp(point.x + (screen.width*1.1) , point.y + (CCRANDOM_0_1() * screen.height)) );
        }
    }
    
    if (preventObstaclesOverlappingObstackes || preventObstaclesOverlappingCoins) {

        Rect rn = RectMake(obstacle->getPosition().x - obstacle->sprite->getContentSize().width * obstacle->obstacle_scale * 0.5 * 2,
                               obstacle->getPosition().y - obstacle->sprite->getContentSize().height * obstacle->obstacle_scale * 0.5,
                               obstacle->sprite->getContentSize().width * obstacle->obstacle_scale * 2,
                               obstacle->sprite->getContentSize().height * obstacle->obstacle_scale);
        
        
        if (preventObstaclesOverlappingObstackes) {
            
            //check for obstacle overlap
            for (int i=0; i < obstacles_info->count(); i++) {
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
            
        }
        
        if (preventObstaclesOverlappingCoins) {
            
            // check for overlap with coins
            for (int i=0; i < coins_info->count(); i++) {
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
            
        }
        
    }

    
    this->addChild( obstacle, character_depth_z - (obstacles_info->count() - typeId) );

    if (obstacles[ kEnemyMax-1 ] != NULL) {
        obstacles[ kEnemyMax-1 ]->destroyPhysics( world );
        this->removeChild(obstacles[ kEnemyMax-1 ], true);
    }
    
    for (int i = kEnemyMax-1; i > 0; i--){
        obstacles[ i ] = obstacles[ i-1 ];
    }
    
    obstacles[ 0 ] =  obstacle;
        
}

void PZGGameFieldScene::addCoin( int typeId ){
    if (coins_info == NULL || coins_info->count() == 0) {
        return;
    }

    PZGCoin *coin = PZGCoin::create( (PZGArtCoins*)coins_info->objectAtIndex( typeId ), world);
    if (coin == NULL) {
        return;
    }

    Size screen = Director::getInstance()->getWinSize();
    
    if (restrictCoins) {
        
        Vec2 point = ccp(screen.width * boundLeft,screen.height * boundUp);
        
        screen.width = screen.width - ((screen.width * boundRight)+point.x);
        screen.height = screen.height - ((screen.height * boundDown)+point.y);
        
        Size spriteSize = coin->sprite->getContentSize();
        
        screen.width-=spriteSize.width+boundRight*kDeviceScale();
        screen.height-=spriteSize.height+boundDown*kDeviceScale();
        
        if (point.x > 0) {
            point.x+=boundLeft*kDeviceScale();
        }
        
        if (point.y > 0) {
            point.y+=boundUp*kDeviceScale();
        }
        
        if (isPortraitMode) {
            coin->setPosition( ccp(point.x + (CCRANDOM_0_1() * screen.width) , point.y + screen.height*1.1) );
        }else{
            coin->setPosition( ccp(point.x + (screen.width*1.1) , point.y + (CCRANDOM_0_1() * screen.height)) );
        }
        
    } else {

        if (isPortraitMode) {
            coin->setPosition( ccp( CCRANDOM_0_1() * screen.width ,  screen.height*1.1) );
        }else{
            coin->setPosition( ccp( screen.width*1.1 , CCRANDOM_0_1() * screen.height) );
        }
    }
    
    
    this->addChild( coin, character_depth_z );
    
    if (coins[ kCoinMax-1 ] != NULL) {
        coins[ kCoinMax-1 ]->destroyPhysics( world );
        this->removeChild(coins[ kCoinMax-1 ], true);
    }
    
    for (int i = kCoinMax-1; i > 0; i--){
        coins[ i ] = coins[ i-1 ];
    }
    
    coins[ 0 ] =  coin;
    
}

void PZGGameFieldScene::update(float dd){
    
    if (dd > 1) {
        dd = 0.1;
    }

    if (tip_counter <= 3 ){
        tip_counter += dd;
        
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
        world->Step(dd, 3, 3);
        
        gf_speed += gf_speed_increaser * dd;
        if (gf_speed > gf_speed_max) {
            gf_speed = gf_speed_max;
        }
        gf_distance += gf_speed * dd;        

        Size size = Director::getInstance()->getWinSize();

        char c[16];
        sprintf( c, "%d", (int)gf_distance);
        distance_label->setString( c );
        
        if (character) {
            character->update( dd );
        }

        
        for (int i=0; i < parallax_n; i++) {
            if (parallax[ i ]) {
                parallax[ i ]->updateWithVelocity(ccp(-gf_speed,-gf_speed), dd*kDeviceScale()*2);
            }
        }
        
        
        //UPDATE SPAWN COUNTERS
        if (obstacles_info) {
            for (int i=0; i < obstacles_info->count(); i++) {
                PZGArtObstacle *info = (PZGArtObstacle*)obstacles_info->objectAtIndex( i );
                obstacles_spawn_counter[ i ] += dd;
                
                if (info->comingRatio <= obstacles_spawn_counter[ i ] && info->comingRatio > 0) {
                    addObstacle( i );
                    obstacles_spawn_counter[ i ] = CCRANDOM_0_1() * (info->comingRatio*0.5);
                }
            }
        }
        
        if (coins_info) {
            for (int i=0; i < coins_info->count(); i++) {
                PZGArtCoins *info = (PZGArtCoins*)coins_info->objectAtIndex( i );
                coin_spawn_counter[ i ] += dd;
                
                if (info->comingRatio <= coin_spawn_counter[ i ] && info->comingRatio > 0) {
                    addCoin( i );
                    coin_spawn_counter[ i ] = CCRANDOM_0_1() * (info->comingRatio*0.5);
                }
            }
        }
        
        //UPDATE ENEMIES
        updateObstacles(dd);
        updateCoins(dd);
        
    }

    
    if (coins_add_counter > 0) {
        coins_add_counter -= dd;
        coins_add_velocity -= dd*3;
        
        Vec2 p = coins_add_label->getPosition();
        coins_add_label->setPosition( ccp( p.x + dd*10 , p.y + dd*coins_add_velocity*100 ) );
        coins_add_label->setOpacity( clampf(coins_add_counter * 255, 0, 255)  );
    }

}

void PZGGameFieldScene::updateObstacles( float dt ){
    float s =  dt*gf_speed*32*kDeviceScale()*2;
    
    if (obstacles_info) {
        for (int i = 0; i < kEnemyMax; i++) {
            if (obstacles[ i ]) {
                obstacles[ i ]->gameSpeed = s;
                
                obstacles[ i ]->update( dt );
                
                if (obstacles[ i ]->deleteReady) {
                    obstacles[ i ]->removeFromParentAndCleanup( true );
                    obstacles[ i ]->destroyPhysics( world );
                    obstacles[ i ] = NULL;
                }
            }
        }
    }
}

void PZGGameFieldScene::updateCoins( float dt ){
    float s =  dt*gf_speed*32*kDeviceScale()*2;
    
    if (coins_info) {
        for (int i = 0; i < kCoinMax; i++) {
            if (coins[ i ]) {
                if (coins[ i ]->disabled == false) {
                    if (isPortraitMode) {
                        coins[ i ]->setPosition( ccp(coins[ i ]->getPosition().x, coins[ i ]->getPosition().y - s) );
                    }
                    else{
                        coins[ i ]->setPosition( ccp(coins[ i ]->getPosition().x - s, coins[ i ]->getPosition().y) );
                    }
                    coins[ i ]->update( dt );
                    
                }
                else{
                    if (coins[ i ]->deleteReady) {
                        coins[ i ]->removeFromParentAndCleanup( true );
                        coins[ i ]->destroyPhysics( world );
                        coins[ i ] = NULL;
                    }
                }
            }
        }
    }
   
}


#pragma mark IAP

void PZGGameFieldScene::upgradeComplete(){
    PZSettingsController* sc = PZSettingsController::shared();
    sc->removeAds = true;
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 8 ); //INDEX 5 is FOR UPGRADE BUTTON
        if (item) {
            item->setOpacity( 0 );
            item->setEnabled( false );
        }
    }
}


void PZGGameFieldScene::kidModePurchased(){
    
    PZSettingsController *sc = PZSettingsController::shared();
    sc->kidMode = true;
    sc->kidModePurchased = true;
    sc->save();
    
    Menu *menu = (Menu*)this->getChildByTag( kBaseMenuItemTag );
    if (menu) {
        MenuItemSprite *item;
        item = (MenuItemSprite*)menu->getChildByTag( 6 ); //KIDMODE BUTTON
        if (item) {
            item->setVisible(false);
        }
    }
}

void PZGGameFieldScene::keyBackClicked(){
    Director *pDirector = Director::getInstance();
    pDirector->popScene();
}
