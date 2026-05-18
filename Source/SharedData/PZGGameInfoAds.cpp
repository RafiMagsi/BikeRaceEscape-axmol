//
//  PZGGameInfoAds.cpp
//  BikeRaceEscapeAxmol
//

#include "PZGGameInfoAds.h"

PZGGameInfoAds* PZGGameInfoAds::createWithDictionary(ax::__Dictionary* dictionary) {
    auto* obj = new PZGGameInfoAds();

    obj->className = dictionary->valueForKey("className");
    obj->key = (ax::__String*)dictionary->valueForKey("key");

    obj->admob_enabled = dictionary->valueForKey("admob_enabled")->boolValue();
    obj->admob_app_id_ios = (ax::__String*)dictionary->objectForKey("admob_app_id_ios");
    obj->admob_app_id_android = (ax::__String*)dictionary->objectForKey("admob_app_id_android");

    obj->admob_banner_id_ios = (ax::__String*)dictionary->objectForKey("admob_banner_id_ios");
    obj->admob_interstitial_id_ios = (ax::__String*)dictionary->objectForKey("admob_interstitial_id_ios");
    obj->admob_rewarded_id_ios = (ax::__String*)dictionary->objectForKey("admob_rewarded_id_ios");

    obj->admob_banner_id_android = (ax::__String*)dictionary->objectForKey("admob_banner_id_android");
    obj->admob_interstitial_id_android = (ax::__String*)dictionary->objectForKey("admob_interstitial_id_android");
    obj->admob_rewarded_id_android = (ax::__String*)dictionary->objectForKey("admob_rewarded_id_android");

    return obj;
}

