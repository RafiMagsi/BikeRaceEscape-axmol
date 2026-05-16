//
//  ContactListener.h
//  PZPlayer
//
//  Created by Nik Rudenko on 6/24/12.
//  Copyright (c) 2012 Kayabit. All rights reserved.
//

#ifndef PZPlayer_ContactListener_h
#define PZPlayer_ContactListener_h

#include "Box2D/Box2D.h"
#include "PZLegacyCompat.h"

class PZGContactListener : public b2ContactListener
{
    
public:
    PZGContactListener();
    ~PZGContactListener();
    
	virtual void BeginContact( b2Contact* contact);
    virtual void EndContact(b2Contact* contact);
	virtual void PreSolve( b2Contact* contact, const b2Manifold* oldManifold);
    
};


#endif
