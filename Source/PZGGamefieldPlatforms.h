//
//  PZGGamefieldPlatforms.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/6/12.
//
//

#ifndef __ProjectMayhem__PZGGamefieldPlatforms__
#define __ProjectMayhem__PZGGamefieldPlatforms__


#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"
#include "SharedData/PZGSharedData.h"

#include "PZGObstacle.h"
#include "PZGCoin.h"
#include "PZGPlatfrom.h"

#include "SharedData/PZGGameInfoLevelItem.h"

#define kMaxBasicRunnerObsctales 32
#define kMaxBasicRunnerCoins 32
#define kMaxRunnerObstaclesTypes 16

#define kMaxPlatfroms 16
#define kNumOFPlatforms 3

//#define kPlatfromTag 75

class PZGGamefieldPlatforms : public ax::Layer {

    PZGGameplayBasicRunner *gameplayInfo;
    
    PZGPlatfrom*        platforms[ kMaxPlatfroms ];
    
    ax::__Array*   levels_info[ kNumOFPlatforms ];
    
    ax::__Array*   obstacles_info;
    PZGObstacle*        obstacles[ kMaxBasicRunnerObsctales ];
    float               obstacles_spawn_counter[ kMaxRunnerObstaclesTypes ];

    
    ax::__Array*   coins_info;
    PZGCoin*            coins[ kMaxBasicRunnerCoins ];
    float               coins_spawn_counter[ kMaxBasicRunnerCoins ];
    
    float length;
    bool prefillPending;

    
    int getCoinIndex( PZGGameInfoLevelItem *item );
    int getObstacleIndex( PZGGameInfoLevelItem *item );
    
public:
    PZGGamefieldPlatforms();
    ~PZGGamefieldPlatforms() override;

    bool isSlowMoveMode;
    
    float speed;
    float distance;
    
    b2World* world;
    
    virtual bool init();
    static ax::Scene* scene();
  
    void reset();
    void update(float dt);
    void addPlatform();

    void addLevel( int platform_type_id );
    void addObstacle( int typeId );
    void addCoins( int typeId );
    
    float platfromsBetweenDistance;
    
    PZGGameInfoLevel* getLevelInfo(float distance, int platform_type_id);
    
    void destoryPhysics();

    // --- read access for the meta features (power-ups, near-miss, revive); see Source/Features ---
    int coinSlots() const { return kMaxBasicRunnerCoins; }
    PZGCoin* coinAt(int i) const { return (i >= 0 && i < kMaxBasicRunnerCoins) ? coins[i] : nullptr; }
    int obstacleSlots() const { return kMaxBasicRunnerObsctales; }
    PZGObstacle* obstacleAt(int i) const { return (i >= 0 && i < kMaxBasicRunnerObsctales) ? obstacles[i] : nullptr; }

    // Surface height of the platform under `x` (scene units). Returns false when `x` is over a gap.
    bool groundYAt(float x, float& outY) const;
    // Slides the world left (never right) so a platform lies under `x`, clears obstacles around it, and
    // returns the surface height there. Used to put the rider back on solid ground after a revive / turbo.
    bool alignForRevive(float x, float& outGroundY);

    LAYER_CREATE_FUNC( PZGGamefieldPlatforms );
};

#endif /* defined(__ProjectMayhem__PZGGamefieldPlatforms__) */
