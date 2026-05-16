#include "CCParallaxScrollOffset.h"

using namespace cocos2d;

// on "init" you need to initialize your instance
bool CCParallaxScrollOffset::init()
{
	if ( !Layer::init() ) return false;
    
    this->setRelVelocity(ccp(0,0));
    this->setScrollOffset(ccp(0,0));
    this->setPosition(ccp(0,0));
    this->setRatio(ccp(0,0));
    return true;
}

CCParallaxScrollOffset* CCParallaxScrollOffset::scrollWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s){
    CCParallaxScrollOffset *offset = CCParallaxScrollOffset::create();
	return (CCParallaxScrollOffset*)offset->initWithNode(node,r,p,s);
}

CCParallaxScrollOffset* CCParallaxScrollOffset::scrollWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s, Vec2 v){
    CCParallaxScrollOffset *offset = CCParallaxScrollOffset::create();
	return (CCParallaxScrollOffset*)offset->initWithNode(node,r,p,s,v);
}

CCParallaxScrollOffset* CCParallaxScrollOffset::initWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s, Vec2 v){
    if(!this) return false;
    this->setRatio(r);
    this->setScrollOffset(s);
    this->setRelVelocity(v);
    this->setOrigPosition(p);
    node->setPosition(p);
    node->setAnchorPoint(ccp(0, 0));
    this->setTheChild(node);
	return this;
}

CCParallaxScrollOffset* CCParallaxScrollOffset::initWithNode(Node *node, Vec2 r, Vec2 p, Vec2 s){
	return this->initWithNode(node, r, p, p, ccp(0,0));
}