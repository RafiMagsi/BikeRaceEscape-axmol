//
//  PZGSoundData.h
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ProjectZero_PZGSoundData_h
#define ProjectZero_PZGSoundData_h

#include <string>
#include "PZLegacyCompat.h"

#define C_SOUND_CLASS_NAME "PZSoundObject"

class PZGSoundData : public ax::Object
{
public:

    PZGSoundData();
    ~PZGSoundData();
    

    const ax::__String * className;
    ax::__String* key;
    int   index;
    
    static PZGSoundData* createWithDictionary(ax::__Dictionary * dictionary);

    void playAsBackgroundMusic();
    void playAsSound( bool looped );
    void stopSound( );
    
    int soundId;
    
    bool isMusic;
    
    const char* getSoundResourcePath();
    void preloadSound();
    
};

#endif
