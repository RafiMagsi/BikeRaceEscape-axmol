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
    
    b2Body*             body;

    ax::Animate*  sprite_death;
    ax::Sprite*  sprite_flash;
    
    float               damage_counter;
    
    float counter;

    
    PZGSoundData *deathSound;
    
public:
 
    ax::Vec2 velocityLinear;
    
    float obstacle_scale;
    ax::Sprite*  sprite;
    
    void setRotation( float angle );
    
    virtual void setScaleX( float scale );
    virtual void setScaleY( float scale );
    
    float health;
    bool  destroyCharacter;
    bool  bouncingAnimation;
    bool  rotationAnimation;
    float rotationVelocity;
    float comingRatio;
    
    bool dead;
    bool deleteReady;
    
    float speed;
    float gameSpeed;
    
    PZGObstacleTrajectory trajectory;
    
    static PZGObstacle* create( PZGArtObstacle* obstInfo, b2World *world );
    void initPhysics( PZGArtObstacle* obstInfo, b2World *world );
    void init(PZGArtObstacle* obstInfo);
    void update( float dt);
    void damage(float power);
    void destroyPhysics( b2World* world );
};


#endif /* defined(__ProjectMayhem__PZGObstacle__) */
