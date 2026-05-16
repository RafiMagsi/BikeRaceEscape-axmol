//
//  PZGGamefieldJumpingPlatforms.h
//  ProjectMayhem
//
//  Created by Amit Srivastava on 08/05/13.
//
//

#ifndef __ProjectMayhem__PZGGamefieldJumpingPlatforms__
#define __ProjectMayhem__PZGGamefieldJumpingPlatforms__

#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"
#include "SharedData/PZGSharedData.h"

#include "PZGObstacle.h"
#include "PZGCoin.h"
#include "PZGPlatfrom.h"

#include "SharedData/PZGGameInfoLevelItem.h"

#define kMax_BasicRunnerObsctales 50
#define kMax_BasicRunnerCoins 50
#define kMax_RunnerObstaclesTypes 16

#define kMax_Platfroms 16
#define kNum_OFPlatforms 3

//#define kPlatfromTag 75

class PZGGamefieldJumpingPlatforms : public ax::Layer {
    
    PZGGameplayJumpingGame *gameplayInfo;
    
    PZGPlatfrom*        platforms[ kMax_Platfroms ];
    
    ax::__Array*   levels_info[ kNum_OFPlatforms ];
    
    ax::__Array*   obstacles_info;
    PZGObstacle*        obstacles[ kMax_BasicRunnerObsctales ];
    float               obstacles_spawn_counter[ kMax_RunnerObstaclesTypes ];
    
    
    ax::__Array*   coins_info;
    PZGCoin*            coins[ kMax_BasicRunnerCoins ];
    float               coins_spawn_counter[ kMax_BasicRunnerCoins ];
    
    float length;
    
    
    int getCoinIndex( PZGGameInfoLevelItem *item );
    int getObstacleIndex( PZGGameInfoLevelItem *item );
    
public:
    PZGGamefieldJumpingPlatforms();
    ~PZGGamefieldJumpingPlatforms() override;
    
    bool isSlowMoveMode;
    
    float speed;
    float distance;
    
    b2World* world;
    
    virtual bool init();
    static ax::Scene* scene();
    
    void reset();
    void update(float dt);
    ax::Rect addPlatform(int pIndex,float dt, ax::Rect rect);
    void addPlatforms(float dt);
    void scroll(float scrollDiff,float dt);
    
    void addLevel(int pIndex, int platform_type_id );
    void addObstacle(int pIndex,  int typeId );
    void addCoins(int pIndex,  int typeId );
    
    float platfromsBetweenDistance;
    
    PZGGameInfoLevel* getLevelInfo(float distance, int platform_type_id);
    
    void destoryPhysics();
    
    ax::Sprite* getPlatformForIndex(int pIndex);
    
    LAYER_CREATE_FUNC( PZGGamefieldJumpingPlatforms );
};

#endif /* defined(__ProjectMayhem__PZGGamefieldJumpingPlatforms__) */
