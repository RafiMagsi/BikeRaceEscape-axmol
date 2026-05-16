//
//  PZGSoundData.cpp
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "PZGSoundData.h"
#include "PZGSharedData.h"

#include "Contants_data.h"
#include "constants.h"

#include "PZLegacyCompat.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    #include "jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h"
#endif

PZGSoundData::PZGSoundData(){
    
}
PZGSoundData::~PZGSoundData(){
    
}

PZGSoundData* PZGSoundData::createWithDictionary(ax::__Dictionary * dictionary){
    
    PZGSoundData *obj = new PZGSoundData();
    
    obj->className = dictionary->valueForKey(SOUND_OBJECT);
    obj->key = (ax::__String*)dictionary->valueForKey( "key" );
    
    obj->index = dictionary->valueForKey("index")->intValue();
    obj->isMusic = dictionary->valueForKey("isMusic")->boolValue();
    
    obj->soundId = -1;
    
    return obj;
}
void PZGSoundData::stopSound( ){
    if (soundId > 0) {
        CocosDenshion::SimpleAudioEngine* audioEngine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        audioEngine->stopEffect(soundId);
        soundId = -1;
    }
}

void PZGSoundData::playAsSound( bool looped ){
    CocosDenshion::SimpleAudioEngine* audioEngine = CocosDenshion::SimpleAudioEngine::sharedEngine();
    
    soundId = audioEngine->playEffect( getSoundResourcePath(), looped);
}

void PZGSoundData::playAsBackgroundMusic(){

    CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic( getSoundResourcePath(), true);
    
}

const char * PZGSoundData::getSoundResourcePath(){
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    char* filename = (char*)malloc(sizeof(char) * 64);
    snprintf(filename, 64, "%s_%d.mp3", key->getCString(), index);
    
#if   CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    return gsd->getFullPath( filename )->getCString();
    
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    // AppDelegate sets search paths to include `Content/Sounds`, so the bare filename resolves correctly.
    return gsd->getFullPath(filename)->getCString();
    
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
    char* out = (char*)malloc(sizeof(char) * 256);
    snprintf(out, 256, "Sounds/%s", filename);
    return out;
    
#endif
}

void PZGSoundData::preloadSound(){
    CocosDenshion::SimpleAudioEngine* audioEngine = CocosDenshion::SimpleAudioEngine::sharedEngine();
    if (isMusic) {
        PZLog( "preloading music" );
        // audioEngine->preloadBackgroundMusic( getSoundResourcePath() );
    }
    else{
        PZLog( "preloading sound" );
        const char *path = getSoundResourcePath();

#if   CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID        
        std::string fullPath( path );
        fullPath.insert(0, "assets/");
        std::string apkPath( getApkPath() );
        bool exist = ax::FileUtils::getInstance()->isFileExistInZip( apkPath.c_str(), fullPath.c_str());
        
        if (exist == false){
            PZLog( "Warning Sound NOT FOUND" );
            return;
        }
#endif
        
        audioEngine->preloadEffect( path );
    }
}
