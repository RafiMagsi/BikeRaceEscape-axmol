//
//  PZGGameInfoIAP.cpp
//  PZPlayer
//
//  Created by Nik Rudenko on 1/23/13.
//
//

#include "PZGGameInfoIAP.h"

PZGGameInfoIAP::PZGGameInfoIAP()
{
    
}
PZGGameInfoIAP::~PZGGameInfoIAP()
{
    
}

PZGGameInfoIAP*  PZGGameInfoIAP::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGGameInfoIAP *obj = new PZGGameInfoIAP();

    obj->className = (ax::__String*)dictionary->valueForKey("className");
    obj->key = (ax::__String*)dictionary->valueForKey("key");

    obj->removeAd_enabled = ((ax::__String*)dictionary->objectForKey("removeAd_enabled"))->boolValue();
    obj->removeAd_id =(ax::__String*)dictionary->objectForKey("removeAd_id");
    if (obj->removeAd_id) {
        obj->removeAd_id->retain();
    }

    
    obj->kidMode_enabled = ((ax::__String*)dictionary->objectForKey("kidMode_enabled"))->boolValue();
    obj->kidMode_id =(ax::__String*)dictionary->objectForKey("kidMode_id");
    if (obj->kidMode_id) {
        obj->kidMode_id->retain();
    }
    
    ax::__String* cstring = NULL;
    
    obj->coinShop_enabled = ((ax::__String*)dictionary->objectForKey("coinShop_enabled"))->boolValue();
    cstring = (ax::__String*)dictionary->objectForKey("coinShop1_value");
    if (cstring) {
        obj->coinShop1_value = cstring->intValue();
    }
    else{
        obj->coinShop1_value = 0;
    }
   
    cstring = (ax::__String*)dictionary->objectForKey("coinShop2_value");
    if (cstring) {
        obj->coinShop2_value = cstring->intValue();
    }
    else{
        obj->coinShop2_value = 0;
    }

    cstring = (ax::__String*)dictionary->objectForKey("coinShop3_value");
    if (cstring) {
        obj->coinShop3_value = cstring->intValue();
    }
    else{
        obj->coinShop3_value = 0;
    }

    obj->coinShop1_id = (ax::__String*)dictionary->objectForKey("coinShop1_id");
    if (obj->coinShop1_id) {
        obj->coinShop1_id->retain();
    }
    obj->coinShop2_id = (ax::__String*)dictionary->objectForKey("coinShop2_id");
    if (obj->coinShop2_id) {
        obj->coinShop2_id->retain();
    }

    obj->coinShop3_id = (ax::__String*)dictionary->objectForKey("coinShop3_id");
    if (obj->coinShop3_id) {
        obj->coinShop3_id->retain();
    }

    obj->storeKey = (ax::__String*)dictionary->objectForKey("storeKey");
    if (obj->storeKey) {
        obj->storeKey->retain();
    }
    
    return obj;
    
}
