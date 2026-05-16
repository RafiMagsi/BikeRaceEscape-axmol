//
//  PZGCoinShopScene.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 1/9/13.
//
//

#ifndef __ProjectMayhem__PZGCoinShopScene__
#define __ProjectMayhem__PZGCoinShopScene__

#include "PZLegacyCompat.h"
//#include "PZGBaseMenuScene.h"
#include "SharedData/PZGSharedData.h"

USING_NS_AX;

class PZGCoinShopScene : public ax::Layer//PZGBaseMenuScene
{
private:
    
public:
    virtual bool init();
	static Scene* scene();
	
    void kidModePurchased();
    
	virtual void menuBackCallback(Ref* pSender);

   	virtual void buyCoinPack1Callback(Ref* pSender);
   	virtual void buyCoinPack2Callback(Ref* pSender);
   	virtual void buyCoinPack3Callback(Ref* pSender);
   	    
    virtual void load(const char* keyName);
    
    virtual void keyBackClicked();
    
    PZGSoundData* buttonClickedSound;
    
    LAYER_CREATE_FUNC( PZGCoinShopScene );
    
    PZGArtInterface* getItemByName( const char* name,  const char* interfaceKey);
};

#endif /* defined(__ProjectMayhem__PZGCoinShopScene__) */
