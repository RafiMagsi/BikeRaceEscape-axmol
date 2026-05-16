#ifndef ParallaxScrollNodeTest_ParallaxScrollNode_h
#define ParallaxScrollNodeTest_ParallaxScrollNode_h

#include "PZLegacyCompat.h"
#include "CCParallaxScrollOffset.h"

using namespace cocos2d;

class CCParallaxScrollNode : public ax::Layer
{
public:
	virtual bool init();
    ~CCParallaxScrollNode();
	CREATE_FUNC(CCParallaxScrollNode);
    
    SpriteBatchNode batch;
    Size _range;
    __Array *_scrollOffsets;
    
    void addChild(Sprite *node, int z, Vec2 r, Vec2 p, Vec2 so);
    void addChild(Sprite *node, int z, Vec2 r, Vec2 p, Vec2 so, Vec2 v);
    
    void removeChild(Sprite *node, bool cleanup);
    void updateWithVelocity(Vec2 vel, float dt);
    void updateWithYPosition(float y, float dt);
    
    void reset();
    
    void addInfiniteScrollWithZ(int z, Vec2 ratio, Vec2 pos, Vec2 dir, Sprite *firstObject, ...);
    void addInfiniteScrollXWithZ(int z, Vec2 ratio, Vec2 pos, Sprite* firstObject, ...);
    void addInfiniteScrollYWithZ(int z,  Vec2 ratio, Vec2 pos, Sprite* firstObject, ...);
    
    void addInfiniteScrollWithObjects(__Array *objects, int z, Vec2 ratio, Vec2 pos, Vec2 dir);
    void addInfiniteScrollWithObjects(__Array *objects, int z, Vec2 ratio, Vec2 pos, Vec2 dir, Vec2 relVel);
    void addInfiniteScrollWithObjects(__Array *objects, int z, Vec2 ratio, Vec2 pos, Vec2 dir, Vec2 relVel, Vec2 padding);
    
};

#endif