#include "CCParallaxScrollNode.h"
#include "constants.h"

#ifndef PTM_RATIO
#define PTM_RATIO 32
#endif

#ifndef SIGN
#define SIGN(x) ((x < 0) ? -1 : (x > 0))
#endif

bool CCParallaxScrollNode::init()
{
	if ( !Layer::init() ) return false;
    
    _scrollOffsets = __Array::create();
    _scrollOffsets->retain();
    Size screen = Director::getInstance()->getWinSize();
    _range = SizeMake(screen.width, screen.height);
    this->setAnchorPoint(ccp(0,0));
    
    return true;
}

CCParallaxScrollNode::~CCParallaxScrollNode()
{
    if (_scrollOffsets) {
        _scrollOffsets->release();
    }
}

// Used with box2d style velocity (m/s where m = 32 pixels), but box2d is not required
void CCParallaxScrollNode::updateWithVelocity(Vec2 vel, float dt)
{
	vel = ccpMult(vel, PTM_RATIO);
    //	CCLog("count: %i", _scrollOffsets->count());
    Ref* object;
    CCARRAY_FOREACH(_scrollOffsets, object)
    {
        CCParallaxScrollOffset* scrollOffset = dynamic_cast<CCParallaxScrollOffset*>(object);
		Vec2 relVel = ccpMult(scrollOffset->getRelVelocity(), PTM_RATIO);
		Vec2 totalVel = ccpAdd(vel, relVel);
		Vec2 offset = ccpCompMult(ccpMult(totalVel, dt), scrollOffset->getRatio());
        
        Node *child = scrollOffset->getTheChild();
		child->setPosition(ccpAdd(child->getPosition(), offset));
        
		if ( (offset.x < 0 && child->getPosition().x + child->getContentSize().width*child->getScaleX()< 0) ||
            (offset.x > 0 && child->getPosition().x > _range.width) ) {
			child->setPosition(ccpAdd(child->getPosition(), ccp(-SIGN(offset.x) * fabs(scrollOffset->getScrollOffset().x), 0)));
		}
        
		// Positive y indicates upward movement in cocos2d
		if ( (offset.y < 0 && child->getPosition().y + child->getContentSize().height*child->getScaleY() < 0) ||
			(offset.y > 0 && child->getPosition().y > _range.height) ) {
			child->setPosition(ccpAdd(child->getPosition(), ccp(0, -SIGN(offset.y) * fabs(scrollOffset->getScrollOffset().y))));
		}
	}
}


void CCParallaxScrollNode::updateWithYPosition(float y, float dt)
{
    
    Ref* object;
    CCARRAY_FOREACH(_scrollOffsets, object)
    {
        CCParallaxScrollOffset* scrollOffset = dynamic_cast<CCParallaxScrollOffset*>(object);
		Node *child = scrollOffset->getTheChild();
		float offset = y * scrollOffset->getRatio().y;
		child->setPosition(ccp(child->getPosition().x, scrollOffset->getOrigPosition().y + offset));
	}
}


void CCParallaxScrollNode::addChild(Sprite *node, int z, Vec2 r, Vec2 p, Vec2 so, Vec2 v)
{
	node->setAnchorPoint(ccp(0,0));
    
	CCParallaxScrollOffset *obj = CCParallaxScrollOffset::scrollWithNode(node, r, p, so, v);
	_scrollOffsets->addObject(obj);
    this->ax::Node::addChild(node, z);
}

void CCParallaxScrollNode::addChild(Sprite *node, int z, Vec2 r, Vec2 p, Vec2 so){
	this->addChild(node,z,r,p,so,ccp(0,0));
}

void CCParallaxScrollNode::removeChild(Sprite *node, bool cleanup)
{
    __Array *dealloc = new __Array();
    dealloc->autorelease();
    
    
    Ref* object;
    CCARRAY_FOREACH(_scrollOffsets, object)
    {
        CCParallaxScrollOffset* scrollOffset = dynamic_cast<CCParallaxScrollOffset*>(object);
		if( scrollOffset->getTheChild() == node){
			dealloc->addObject(scrollOffset);
			break;
		}
	}
    _scrollOffsets->removeObjectsInArray(dealloc);
}

void CCParallaxScrollNode::addInfiniteScrollWithObjects(__Array *objects, int z, Vec2 ratio, Vec2 pos, Vec2 dir, Vec2 relVel, Vec2 padding)
{
	// NOTE: corrects for 1 pixel at end of each sprite to avoid thin lines appearing
    
	// Calculate total width and height
	float totalWidth = 0;
	float totalHeight = 0;
    
    
    Ref* object;
    CCARRAY_FOREACH(objects, object)
    {
        Sprite* sprite = dynamic_cast<Sprite*>(object);
		totalWidth += sprite->getContentSize().width * sprite->getScaleX() + dir.x * padding.x;
		totalHeight += sprite->getContentSize().height * sprite->getScaleY() + dir.y * padding.y;
	}
    
	// Position objects, add to parallax
	Vec2 currPos = pos;
    CCARRAY_FOREACH(objects, object)
    {
        Sprite* sprite = dynamic_cast<Sprite*>(object);
		this->addChild(sprite, z, ratio, currPos, ccp(totalWidth, totalHeight), relVel);
		Vec2 nextPosOffset = ccp(dir.x * (sprite->getContentSize().width*sprite->getScaleX() + padding.x),
                                    dir.y * (sprite->getContentSize().height*sprite->getScaleY() + padding.y));
		currPos = ccpAdd(currPos, nextPosOffset);
	}
}

void CCParallaxScrollNode::addInfiniteScrollWithObjects(__Array *objects, int z, Vec2 ratio, Vec2 pos, Vec2 dir, Vec2 relVel)
{
//	this->addInfiniteScrollWithObjects(objects, z, ratio, pos, dir, relVel, ccp(-4*CC_CONTENT_SCALE_FACTOR()*kDeviceScale(),-4*CC_CONTENT_SCALE_FACTOR() * kDeviceScale()));
    
    this->addInfiniteScrollWithObjects(objects, z, ratio, pos, dir, relVel, ccp(-1*kDeviceScale(),-1*kDeviceScale()));
}

void CCParallaxScrollNode::addInfiniteScrollWithObjects(__Array *objects, int z, Vec2 ratio, Vec2 pos, Vec2 dir)
{
	this->addInfiniteScrollWithObjects(objects, z, ratio, pos, dir, ccp(0,0));
}

void CCParallaxScrollNode::reset(){
    Ref* object;
    CCARRAY_FOREACH(_scrollOffsets, object)
    {
        CCParallaxScrollOffset* scrollOffset = dynamic_cast<CCParallaxScrollOffset*>(object);
        Node *child = scrollOffset->getTheChild();
		child->setPosition(  scrollOffset->getOrigPosition() );
	}
}

void CCParallaxScrollNode::addInfiniteScrollXWithZ(int z, Vec2 ratio, Vec2 pos, Sprite* firstObject, ...){
	va_list args;
    va_start(args, firstObject);
    
	__Array* argArray = new __Array();
	for (Sprite *arg = firstObject; arg != NULL; arg = va_arg(args, Sprite*)) {
		argArray->addObject(arg);
	}
	va_end(args);
    
	this->addInfiniteScrollWithObjects(argArray, z, ratio, pos, ccp(1,0));
}

void CCParallaxScrollNode::addInfiniteScrollYWithZ(int z, Vec2 ratio, Vec2 pos, Sprite* firstObject, ...){
	va_list args;
    va_start(args, firstObject);
    
	__Array* argArray = new __Array();
	for (Sprite *arg = firstObject; arg != NULL; arg = va_arg(args, Sprite*)) {
		argArray->addObject(arg);
	}
	va_end(args);
    
	this->addInfiniteScrollWithObjects(argArray, z, ratio, pos, ccp(0,1));
}

void CCParallaxScrollNode::addInfiniteScrollWithZ(int z, Vec2 ratio, Vec2 pos, Vec2 dir, Sprite* firstObject, ...){
	va_list args;
    va_start(args, firstObject);
    
	__Array* argArray = new __Array();
	for (Sprite *arg = firstObject; arg != NULL; arg = va_arg(args, Sprite*)) {
		argArray->addObject(arg);
	}
	va_end(args);
    
	this->addInfiniteScrollWithObjects(argArray, z, ratio, pos, dir);
}