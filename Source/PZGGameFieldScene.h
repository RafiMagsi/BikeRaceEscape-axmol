//
//  PZGGameFieldScene.h
//  PZPlayer
//
//  Created by Nik Rudenko on 6/19/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#ifndef PZPlayer_PZGGameFieldScene_h
#define PZPlayer_PZGGameFieldScene_h

#include "PZLegacyCompat.h"
#include "PZLegacyInputLayer.h"
#include "Box2D/Box2D.h"

#include "GLESDebugDraw.h"
#include "GLESDEbugDrawLayer.h"

#include "PZSettingsController.h"
#include "PZGBaseMenuScene.h"

#include "PZGCoin.h"
#include "PZGObstacle.h"
#include "PZGCharacter.h"

#include "Parallax/CCParallaxScrollNode.h"
#include "SharedData/PZGGameInfoGeneral.h"
#include "SharedData/PZGArtBackgroundPlay.h"

#define kCoinType 16
#define kEnemyType 16
#define kCoinMax 40
#define kEnemyMax 32

#define kDebugSceneTag 565

#define kPauseMenuTag 20
#define kBACKGROUND_MAX_LAYERS 16

struct Settings
{
	Settings() :
	hz(60.0f),
	velocityIterations(10),
	positionIterations(4),
	drawStats(0),
	drawShapes(1),
	drawJoints(1),
	drawAABBs(0),
	drawPairs(0),
	drawContactPoints(0),
	drawContactNormals(0),
	drawContactForces(0),
	drawFrictionForces(0),
	drawCOMs(0),
	enableWarmStarting(1),
	enableContinuous(1),
	pause(0),
	singleStep(0)
	{}
	
	float32 hz;
	int32 velocityIterations;
	int32 positionIterations;
	int32 drawShapes;
	int32 drawJoints;
	int32 drawAABBs;
	int32 drawPairs;
	int32 drawContactPoints;
	int32 drawContactNormals;
	int32 drawContactForces;
	int32 drawFrictionForces;
	int32 drawCOMs;
	int32 drawStats;
	int32 enableWarmStarting;
	int32 enableContinuous;
	int32 pause;
	int32 singleStep;
};


class PZGGameFieldScene : public PZGBaseMenuScene
{
    
    bool isPortraitMode;

public:
    
    virtual void kidModePurchased();
    virtual void upgradeComplete();
    
    
    float                     tip_counter;
    Sprite*                 tip1;
    Sprite*                 tip2;
    
    MenuItemSprite*       pauseButton;
    
    PZGSoundData*           coin_pickup_sound;
    PZGSoundData*           gameStartSound;
    
    ax::Label* coins_label;
    ax::Label* coins_add_label;
    
    float                   coins_add_counter;
    float                   coins_add_velocity;
    
    
    char                   coins_picup_sound[64];
    
    
    ax::__Array* backgroundLayers;
    CCParallaxScrollNode *parallax[ kBACKGROUND_MAX_LAYERS ];
    int parallax_n;
    
    ax::Label* distance_label;
    
    b2World *world;
    PZGCharacter*   character;
    int             character_depth_z;
    
    bool gameOver;
    bool gamePause;
    
    virtual bool init();
    virtual void initPhysics( Vec2 gravity );

    virtual void load(const char* keyName);   
    
    static PZGGameFieldScene* shared();
    
    float   gf_distance;
    int     gf_coins;
    float   gf_speed;
    float   gf_speed_max;
    float   gf_speed_increaser;
    
    float boundUp;
    float boundRight;
    float boundDown;
    float boundLeft;
    
	static ax::Scene* scene();
    
    void removePauseMenu();
    void gameOverCallBack( bool _isGameOver);
    void coinPickup( PZGCoin* coin );
    void gameRestart();
    
    virtual void pauseMenuCallback( Ref* pSender );
    virtual void update(float dt);

    virtual void reset();
    
    ax::__Array*   obstacles_info;
    PZGObstacle*        obstacles[ kEnemyMax ];
    float               obstacles_spawn_counter[ kEnemyType ];
    
    ax::__Array*   coins_info;
    PZGCoin*            coins[ kCoinMax ];
    float               coin_spawn_counter[ kCoinType ];
    int                 coin_remove_id;
    
    virtual void addObstacle( int typeId );
    virtual void updateObstacles( float dt );
    
    virtual void addCoin( int typeId );
    virtual void updateCoins( float dt );
    
    virtual void keyBackClicked();
    
    void setDebugModeEnabled(bool enabled);
    
    bool restrictObstacles; // To restrict obstacles within play area
    bool restrictCoins;     // To restrict coins within play area
    
    bool preventObstaclesOverlappingObstackes; //To Prevent obstacles overlapping
    bool preventObstaclesOverlappingCoins; //To Prevent obstacles overlapping
    
    Rect playArea;
    
	LAYER_CREATE_FUNC( PZGGameFieldScene );
};


#endif
