#ifndef ParallaxScrollNodeTest_CCParallaxScrollOffset_h
#define ParallaxScrollNodeTest_CCParallaxScrollOffset_h

#include "PZLegacyCompat.h"
using namespace cocos2d;

class CCParallaxScrollOffset : public ax::Layer
{
public:
	virtual bool init();
    CREATE_FUNC(CCParallaxScrollOffset);
    
    CC_SYNTHESIZE(Vec2, scrollOffset, ScrollOffset);
    CC_SYNTHESIZE(Vec2, origPosition, OrigPosition);
    CC_SYNTHESIZE(Vec2, relVelocity, RelVelocity);
    CC_SYNTHESIZE(Vec2, ratio, Ratio);
    CC_SYNTHESIZE(Vec2, buffer, Buffer);
    CC_SYNTHESIZE(Node*, theChild, TheChild);
    
    static CCParallaxScrollOffset* scrollWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s);
    static CCParallaxScrollOffset* scrollWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s, Vec2 v);
    CCParallaxScrollOffset* initWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s, Vec2 v);
    CCParallaxScrollOffset* initWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s);
};

#endif