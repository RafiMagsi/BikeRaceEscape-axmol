//
//  PZGGameInfoAds.h
//  BikeRaceEscapeAxmol
//

#pragma once

#include "PZLegacyCompat.h"

#define C_GAMEINFO_ADS_CLASS_NAME "PZGameInfoAds"

class PZGGameInfoAds : public ax::Object {
public:
    ax::__String* className{nullptr};
    ax::__String* key{nullptr};

    // AdMob
    bool admob_enabled{false};
    ax::__String* admob_app_id_ios{nullptr};
    ax::__String* admob_app_id_android{nullptr};
    ax::__String* admob_banner_id_ios{nullptr};
    ax::__String* admob_interstitial_id_ios{nullptr};
    ax::__String* admob_rewarded_id_ios{nullptr};
    ax::__String* admob_banner_id_android{nullptr};
    ax::__String* admob_interstitial_id_android{nullptr};
    ax::__String* admob_rewarded_id_android{nullptr};

    static PZGGameInfoAds* createWithDictionary(ax::__Dictionary* dictionary);
};

