#ifndef __MAINMENU_SCENE_H__
#define __MAINMENU_SCENE_H__


#include "PZLegacyCompat.h"
#include "PZGBaseMenuScene.h"
#include "PZGCoinShopScene.h"

#include "SharedData/PZGSharedData.h"


USING_NS_AX;

class PZGMainMenuScene : public PZGBaseMenuScene
{
private:
    
public:

	static ax::Scene* scene();

    virtual bool init();
	
	// a selector callback
	virtual void menuStartGameCallback(Ref* pSender);
    virtual void menuGameCenterCallback(Ref* pSender);
    virtual void menuRestoreCallback(Ref* pSender);

    virtual void offKidModeCallback(Ref* pSender);
    
    virtual void kidModePurchased();
    virtual void upgradeComplete();

    virtual void keyBackClicked();
    
    virtual void update(float dt);
    virtual void load(const char* keyName);
    virtual void moreGames();
    
	// implement the "static node()" method manually
	LAYER_CREATE_FUNC( PZGMainMenuScene )
    
    virtual void onEnterTransitionDidFinish();
    
};

#endif // __MAINMENU_SCENE_H__
