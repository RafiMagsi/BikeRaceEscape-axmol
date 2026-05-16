//
//  PZGGameInfoIAP.h
//  PZPlayer
//
//  Created by Nik Rudenko on 1/23/13.
//
//

#ifndef __PZPlayer__PZGGameInfoIAP__
#define __PZPlayer__PZGGameInfoIAP__

#include <iostream>
#include "PZLegacyCompat.h"

#define C_GAMEINFO_IAP_CLASS_NAME "PZGameInfoIAP"

class PZGGameInfoIAP : public ax::Object
{
public:
    
    PZGGameInfoIAP();
    ~PZGGameInfoIAP();
    
    ax::__String * className;
    ax::__String * key;
    
    bool                    removeAd_enabled;
    ax::__String*      removeAd_id;

    bool                    kidMode_enabled;
    ax::__String*      kidMode_id;

    bool                    coinShop_enabled;
    int                     coinShop1_value;
    int                     coinShop2_value;
    int                     coinShop3_value;
    
    ax::__String*      coinShop1_id;
    ax::__String*      coinShop2_id;
    ax::__String*      coinShop3_id;
    
    ax::__String*      storeKey;
    
    static PZGGameInfoIAP* createWithDictionary(ax::__Dictionary * dictionary);
    
};

#endif /* defined(__PZPlayer__PZGGameInfoIAP__) */
