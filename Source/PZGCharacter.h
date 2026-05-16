//
//  Player.h
//  ProjectZero
//
//  Created by Nik Rudenko on 8/25/12.
//
//

#ifndef __ProjectZero__PZGCharacter__
#define __ProjectZero__PZGCharacter__

#include "PZLegacyCompat.h"
#include "Box2D/Box2D.h"
#include "SharedData/PZGArtCharacter.h"
#include "SharedData/PZGSoundData.h"

#define kCharacterTag 203

USING_NS_AX;

#define kPlayerMaxBullets 8

class PZGCharacter : public ax::Node
{
private:
    
    float       character_scale;
    int         playerId;
    int         player_n;
    
    bool        bouncing;
    float       bouncing_counter;

    Sprite*   s_fire;
    Sprite*   s_bullet[ kPlayerMaxBullets ];
    b2Body*     p_bullet[ kPlayerMaxBullets ];
    Vec2     bullet_velocity[ kPlayerMaxBullets ];

    ax::Animate*  s_body_jump;
    ax::Animate*  s_body_death;
    ax::Animate*  s_body_animation;
    
    bool        bullet_out;

    float counter;
        
public:
    PZGCharacter();
    PZGSoundData* shootingSound;
    
    b2Body*     p_body;
    Sprite*   s_body;
        
    Vec2     loc;
    Vec2     speed;
    bool        rotating;
    bool        coinPicked;
    
    float       angle;
    float       shooting_angle;
    bool        shooting;

    float       jump_height;
    bool        jump_permanently;
    
    float       fire_speed;
    float       bullet_speed;
    
    void        setLoc( Vec2 );
    void        setAngle( float );
    void        setShootingAngle( float );
    
    void        jump();
    void        jumpEnd();
    float       jump_timer;
    bool        jump_enabled;
    
    void        jump_grounded();
    bool        jump_nongrounded;
    float       jump_nongrounded_timer;
    int         jump_counter;
    
    static PZGCharacter* create(int playerId, b2World* world, bool gravityAffected);
    
    void init( PZGArtCharacter* artCharacter );
    void initPhysics(PZGArtCharacter* obstInfo, b2World* world, bool gravityAffected );
    void update(float dd);
    
    void removeBullet(b2Body* body);
    void dead();
    void reset();

    ~PZGCharacter() override;
    
};

#endif /* defined(__ProjectZero__PZGCharacter__) */
