//
//  BaseMenuScene.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 12/13/12.
//
//

#ifndef __ProjectMayhem__BaseMenuScene__
#define __ProjectMayhem__BaseMenuScene__

#include "PZLegacyCompat.h"
#include "PZLegacyInputLayer.h"
#include <iostream>

#include "SharedData/PZGSharedData.h"


class PZGBaseMenuScene : public PZLegacyLayer
{

public:
    
    PZGSoundData* buttonClickedSound;
    
    virtual bool init();
    
    void load(const char* keyName);
    
	static ax::Scene* scene();
	LAYER_CREATE_FUNC( PZGBaseMenuScene );

    PZGArtInterface* getItemByName( const char* name,  const char* interfaceKey);
    
    
    virtual void menuUpgradeCallback(Ref* pSender);
    virtual void coinShopCallback(Ref* pSender);
    virtual void buyKidModeCallback(Ref* pSender);
    virtual void offKidModeCallback(Ref* pSender);    
    
    virtual void kidModePurchased();
    virtual void upgradeComplete();
    
};


#endif /* defined(__ProjectMayhem__BaseMenuScene__) */
