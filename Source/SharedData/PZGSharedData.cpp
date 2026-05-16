//
//  PZSharedData_cpp.cpp
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "PZLegacyCompat.h"
#include <iostream>
#include "PZGSharedData.h"
#include "Contants_data.h"
#include "constants.h"
#include <string>

USING_NS_AX;

namespace {
static void pz_addToBucket(ax::__Dictionary* store, ax::Object* object, const char* key) {
    if (!store || !object) return;
    if (!key || !*key) {
        AXLOGE("PZGSharedData: skip object with empty key (type={})", typeid(*object).name());
        return;
    }

    if (auto* arr = (ax::__Array*)store->objectForKey(key)) {
        arr->addObject(object);
        return;
    }

    auto* arr = ax::__Array::create();
    arr->addObject(object);
    store->setObject(arr, key);
}
} // namespace

PZGSharedData::PZGSharedData()
{
    synchronized = false;
    
    artResource = __Dictionary::create();
    gameInfoResource = __Dictionary::create();
    soundResource = __Dictionary::create();
    
}
PZGSharedData::~PZGSharedData()
{
    
}

static PZGSharedData* s_pSharedData= NULL;

PZGSharedData*  PZGSharedData::sharedInstanse()
{
    if (s_pSharedData == NULL)
    {
        s_pSharedData = new PZGSharedData();
    }
    return s_pSharedData;
}

void PZGSharedData::destroyInstanse(){
    s_pSharedData->PZGSharedData::~PZGSharedData();
    s_pSharedData = NULL;
}

void PZGSharedData::setWorkingURL(const char* url){
//    printf(" URL: %s \n", url);
    workingURL = ax::__String::create( std::string(url) );
    workingURL->retain();
}

void PZGSharedData::setDocumentURL(const char* url){
//    printf(" URL: %s \n", url);
    documentURL = ax::__String::create( std::string(url) );
    documentURL->retain();
}

ax::__String* PZGSharedData::getFullPath(const char* name){
    ax::__String *s;
    
#if   CC_TARGET_PLATFORM == CC_PLATFORM_MAC
    s = ax::__String::createWithFormat("%s/%s", workingURL->getCString(), name);
    
#else
    const auto fullPath = ax::FileUtils::getInstance()->fullPathForFilename(name);
    s = __String::createWithFormat("%s", fullPath.c_str());
#endif
    
    return s;
}

void PZGSharedData::readDataFromFile()
{

    if (synchronized == true) {
        return;
    }

    // Use just the filename and let FileUtils resolve it via search paths.
    // Don't use getFullPath() as it returns an absolute path which FileUtils::getValueMapFromFile()
    // doesn't handle correctly - it expects relative paths to resolve via search paths.
    const char* filename = "CompleteGameInfo.plist";
    auto* fileUtils = ax::FileUtils::getInstance();

    // Check if file exists using search paths
    const auto fullPath = fileUtils->fullPathForFilename(filename);
    if (fullPath.empty() || !fileUtils->isFileExist(fullPath)) {
        AXLOGE("PZGSharedData::readDataFromFile file not found: {}", filename);
        AXLOGI("Current search paths:");
        for (const auto& path : fileUtils->getSearchPaths()) {
            AXLOGI("  - {}", path);
        }
        synchronized = true;  // Mark as attempted to avoid retry loop
        return;
    }

    AXLOGI("Loading plist from: {}", fullPath);

    // Pass just the filename - FileUtils will use search paths to find it
    AXLOGI("About to parse plist: {}", filename);
    __Dictionary *gamePlist = ax::__Dictionary::createWithContentsOfFile(filename);
    if (!gamePlist) {
        AXLOGE("PZGSharedData::readDataFromFile failed to parse plist at: {}", fullPath);
        synchronized = true;  // Mark as attempted to avoid retry loop
        return;
    }
    AXLOGI("Plist parsed successfully");

    AXLOGI("Looking for 'completeInfo' key...");
    __Array * completeGameArray = (ax::__Array *)gamePlist->objectForKey("completeInfo");
    if (!completeGameArray) {
        AXLOGE("PZGSharedData::readDataFromFile missing key 'completeInfo' in: {}", filename);
        synchronized = true;  // Mark as attempted to avoid retry loop
        return;
    }
    AXLOGI("Found completeInfo array with {} items", completeGameArray->count());


    for (int i =0; i< completeGameArray->count(); i++){
        __Dictionary* dict = (__Dictionary*)completeGameArray->objectAtIndex(i);
        
        __String *className =(__String *)dict->objectForKey( OBJECT_CLASS_NAME );
       
        // PZArtBackground
        if ( className->isEqual( __String::create( C_ART_BACKGROUND_CLASS_NAME ) ) ) {
            PZGArtBackground *artObject = PZGArtBackground::createWithDictionary(dict);
            if (artObject && artObject->key) {
                __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
                if ( arr ){
                    ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
                }
                else{
                    arr = __Array::create( );
                    arr->addObject( artObject );
                    artResource->setObject(arr, artObject->key->getCString());
                }
            }
        }else
        // PZArtBackground
        if ( className->isEqual( __String::create( C_ART_BACKGROUNDPLAY_CLASS_NAME ) ) ) {
            PZGArtBackgroundPlay *artObject = PZGArtBackgroundPlay::createWithDictionary(dict);
            if (artObject && artObject->key) {
                __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
                if ( arr ){
                    ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
                }
                else{
                    arr = __Array::create( );
                    arr->addObject( artObject );
                    artResource->setObject(arr, artObject->key->getCString());
                }
            }
        }else
        // PZArtCharacter
        if ( className->isEqual( __String::create( C_ART_CHARACTER_CLASS_NAME ) ) ){
            PZGArtCharacter *artObject = PZGArtCharacter::createWithDictionary(dict);
            if (artObject && artObject->key) {
                __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
                if ( arr ){
                    ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
                }
                else{
                    arr = __Array::create( );
                    arr->addObject( artObject );
                    artResource->setObject(arr, artObject->key->getCString());
                }
            }
        }else
        // PZArtObstacle
        if ( className->isEqual( __String::create( C_ART_OBSTACLE_CLASS_NAME ) ) ){
            PZGArtObstacle *artObject = PZGArtObstacle::createWithDictionary(dict);
            if (artObject && artObject->key) {
                __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
                if ( arr ){
                    ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
                }
                else{
                    arr = __Array::create( );
                    arr->addObject( artObject );
                    artResource->setObject(arr, artObject->key->getCString());
                }
            }
        }else
        // PZArtCoins
        if ( className->isEqual( __String::create( C_ART_COINS_CLASS_NAME ) ) ){
            PZGArtCoins *artObject = PZGArtCoins::createWithDictionary(dict);
            if (artObject && artObject->key) {
                __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
                if ( arr ){
                    ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
                }
                else{
                    arr = __Array::create( );
                    arr->addObject( artObject );
                    artResource->setObject(arr, artObject->key->getCString());
                }
            }
        }else
        // PZArtInterface
        if ( className->isEqual( __String::create( C_ART_INTERFACE_CLASS_NAME ) ) ){
            PZGArtInterface *artObject = PZGArtInterface::createWithDictionary(dict);
            if (artObject && artObject->key) {
                __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
                if ( arr ){
                    ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
                }
                else{
                    arr = __Array::create( );
                    arr->addObject( artObject );
                    artResource->setObject(arr, artObject->key->getCString());
                }
            }
        }else
        // PZSoundObject
        if ( className->isEqual( __String::create( C_SOUND_CLASS_NAME ) ) ){
            PZGSoundData *object = PZGSoundData::createWithDictionary(dict);
            if (object && object->key) {
                __Array* arr = ( __Array* )soundResource->objectForKey( object->key->getCString() );
                if ( arr ){
                    object->index = arr->count();
                    ((__Array*)soundResource->objectForKey( object->key->getCString() ))->addObject( object );
                }
                else{
                    arr = __Array::create( );
                    object->index = 0;
                    arr->addObject( object );
                    soundResource->setObject(arr, object->key->getCString());
                }
            }
        }else
        // PZGameplayBasicRunner
        if ( className->isEqual( __String::create( C_GAMEPLAY_BASICRUNNER_CLASS_NAME ) ) ){
            PZGGameplayBasicRunner *obj = PZGGameplayBasicRunner::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplayBasicAvoidance
        if ( className->isEqual( __String::create( C_GAMEPLAY_BASICAVOIDANCE_CLASS_NAME ) ) ){
            PZGGameplayBasicAvoidance *obj = PZGGameplayBasicAvoidance::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplayHelicopterGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_HELICOPTER_CLASS_NAME ) ) ){
            PZGGameplayHelicopterGame *obj = PZGGameplayHelicopterGame::createWithDictionary(dict);          
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplaySpaceShooter
        if ( className->isEqual( __String::create( C_GAMEPLAY_SPACESHOOTER_CLASS_NAME ) ) ){
            PZGGameplaySpaceShooter *obj = PZGGameplaySpaceShooter::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplayShooterHelicopter
        if ( className->isEqual( __String::create( C_GAMEPLAY_SHOOTINGHELICOPTER_CLASS_NAME ) ) ){
            PZGGameplayShootingHelicopter *obj = PZGGameplayShootingHelicopter::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplayShootingRunner
        if ( className->isEqual( __String::create( C_GAMEPLAY_SHOOTINGRUNNER_CLASS_NAME ) ) ){
            PZGGameplayShootingRunner *obj = PZGGameplayShootingRunner::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplayTiltAvoidance
        if ( className->isEqual( __String::create( C_GAMEPLAY_TILTAVOIDANCE_CLASS_NAME ) ) ){
            PZGGameplayTiltAvoidance *obj = PZGGameplayTiltAvoidance::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplayRoundShooter
        if ( className->isEqual( __String::create( C_GAMEPLAY_ROUNDSHOOTER_CLASS_NAME ) ) ){
            PZGGameplayRoundShooter *obj = PZGGameplayRoundShooter::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplaySlowMoveRunner
        if ( className->isEqual( __String::create( C_GAMEPLAY_SLOWMOVERUNNER_CLASS_NAME ) ) ){
            PZGGameplaySlowMoveRunner *obj = PZGGameplaySlowMoveRunner::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameplayJumpingGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_JUMPING_CLASS_NAME ) ) ){
            PZGGameplayJumpingGame *obj = PZGGameplayJumpingGame::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGGameplayMegaJumpGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_MEGAJUMP_CLASS_NAME ) ) ){
            PZGGameplayMegaJumpGame *obj = PZGGameplayMegaJumpGame::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGGameplayRacingGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_RACINGGAME_CLASS_NAME ) ) ){
            PZGGameplayRacingGame *obj = PZGGameplayRacingGame::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameInfoGeneral
        if ( className->isEqual( __String::create( C_GAMEINFO_GENERAL_CLASS_NAME ) ) ){
            PZGGameInfoGeneral *obj = PZGGameInfoGeneral::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameInfoIAP
        if ( className->isEqual( __String::create( C_GAMEINFO_IAP_CLASS_NAME ) ) ){
            PZGGameInfoIAP *obj = PZGGameInfoIAP::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }else
        // PZGameInfoLevel
        if ( className->isEqual( __String::create( C_GAMEINFO_LEVEL_CLASS_NAME ) ) ){
            PZGGameInfoLevel *obj = PZGGameInfoLevel::createWithDictionary(dict);
            pz_addToBucket(gameInfoResource, obj, (obj && obj->key) ? obj->key->getCString() : nullptr);
        }
    }
    
    artResource->retain();
    gameInfoResource->retain();
    soundResource->retain();
    
    synchronized = true;
}
