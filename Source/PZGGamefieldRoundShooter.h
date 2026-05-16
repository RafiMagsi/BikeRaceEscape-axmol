//
//  PZGamefieldRoundShooter.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/9/13.
//
//

#ifndef __ProjectMayhem__PZGamefieldRoundShooter__
#define __ProjectMayhem__PZGamefieldRoundShooter__

#include <iostream>
#include "PZGGameFieldScene.h"

class PZGGamefieldRoundShooter : public PZGGameFieldScene{
    
    Touch*           fire_touch;
    Touch*           move_touch;
    
    Vec2            coins_vector[ kCoinMax ];
    Vec2            obstacles_vector[ kEnemyMax ];
    
public:
    virtual bool init();
    static ax::Scene* scene();
    
    LAYER_CREATE_FUNC( PZGGamefieldRoundShooter );
    
    ax::Vec2   player_target;
    ax::Vec2   player_start;
    ax::Vec2   player_target_offset;
    
    ax::Vec2   fire_target;
    ax::Vec2   fire_target_offset;

    Vec2 setOffScreenPosition( Vec2 pos,  Size size);
        
    virtual void update(float dt);
    
    virtual void updateObstacles( float dt );
    virtual void addObstacle( int typeId );
    virtual void updateCoins( float dt );
    virtual void addCoin( int typeId );
    
    virtual void reset();
    
    virtual void ccTouchesBegan(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesMoved(ax::__Set *pTouch, ax::Event *pEvent);
    virtual void ccTouchesEnded(ax::__Set *pTouch, ax::Event *pEvent);
};
#endif /* defined(__ProjectMayhem__PZGamefieldRoundShooter__) */
