//
//  PZGObstacle.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/10/12.
//
//

#ifndef __ProjectMayhem__PZGObstacle__
#define __ProjectMayhem__PZGObstacle__

#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"
#include "SharedData/PZGArtObstacle.h"
#include "SharedData/PZGSoundData.h"
#include "SharedData/PZGSharedData.h"

#define kObstacleTag 202
#define kMaxObstacleDeathSounds 3

enum PZGObstacleTrajectory {
    
    PZGObstacleTrajectoryLeftLinear       = 1,
    PZGObstacleTrajectoryLeftDiagonalUp   = 2,
    PZGObstacleTrajectoryLeftDiagonalDown = 3,
    PZGObstacleTrajectoryUp               = 4,
    PZGObstacleTrajectoryDown             = 5,
    PZGObstacleTrajectoryRightLinear      = 6,
    PZGObstacleTrajectoryRightDiagonalUp  = 7,
    PZGObstacleTrajectoryRightDiagonalDown= 8,
    PZGObstacleTrajectoryRandom           = 9,

};

class PZGObstacle : public ax::Node
{
    
    b2Body*             body = nullptr;

    ax::Animate*  sprite_death = nullptr;
    ax::Sprite*  sprite_flash = nullptr;
    
    float               damage_counter = 0.f;
    
    float counter = 0.f;

    
    PZGSoundData *deathSound = nullptr;
    
public:
 
    ax::Vec2 velocityLinear = ax::Vec2::ZERO;
    
    float obstacle_scale = 1.f;
    ax::Sprite*  sprite = nullptr;
    
    void setRotation( float angle );
    
    virtual void setScaleX( float scale );
    virtual void setScaleY( float scale );
    
    float health = 1.f;
    bool  destroyCharacter = false;
    bool  bouncingAnimation = false;
    bool  rotationAnimation = false;
    float rotationVelocity = 0.f;
    float comingRatio = 0.f;
    
    bool dead = false;
    bool deleteReady = false;
    bool nearMissDone = false;   // set by PZRunDirector once the near-miss check for this obstacle ran
    
    float speed = 0.f;
    float gameSpeed = 0.f;

    // Relative movement on top of world scrolling. The bounded offset preserves the authored
    // jump location while making a running creature visibly move and keeping Box2D in sync.
    float patrolDistance = 0.f;
    float patrolSpeed = 0.f;
    float patrolPause = 0.25f;
    float patrolOffset = 0.f;
    float patrolWait = 0.f;
    float patrolDirection = -1.f;
    
    PZGObstacleTrajectory trajectory = PZGObstacleTrajectoryLeftLinear;
    
    static PZGObstacle* create( PZGArtObstacle* obstInfo, b2World *world );
    void initPhysics( PZGArtObstacle* obstInfo, b2World *world );
    void init(PZGArtObstacle* obstInfo);
    void update( float dt);
    void damage(float power);

    // Sprite bounds in the parent's coordinate space (includes node scale/rotation of the placed chunk item).
    ax::Rect worldBox() const;
    void destroyPhysics( b2World* world );
};


#endif /* defined(__ProjectMayhem__PZGObstacle__) */
