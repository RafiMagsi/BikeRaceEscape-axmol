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
    s = __String::createWithFormat( "%s", ax::FileUtils::getInstance()->fullPathFromRelativePath( name ) );
#endif
    
    return s;
}

void PZGSharedData::readDataFromFile()
{
    
    if (synchronized == true) {
        return;
    }
    
    const char *gameFilePath = getFullPath("CompleteGameInfo.plist")->getCString();
    
    PZLog(gameFilePath);
    
    __Dictionary *gamePlist = ax::__Dictionary::createWithContentsOfFile( gameFilePath );
    __Array * completeGameArray = (ax::__Array *)gamePlist->objectForKey("completeInfo");


    for (int i =0; i< completeGameArray->count(); i++){
        __Dictionary* dict = (__Dictionary*)completeGameArray->objectAtIndex(i);
        
        __String *className =(__String *)dict->objectForKey( OBJECT_CLASS_NAME );
       
        // PZArtBackground
        if ( className->isEqual( __String::create( C_ART_BACKGROUND_CLASS_NAME ) ) ) {
            PZGArtBackground *artObject = PZGArtBackground::createWithDictionary(dict);
            __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
            if ( arr ){
                ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
            }
            else{
                arr = __Array::create( );
                arr->addObject( artObject );
                artResource->setObject(arr, artObject->key->getCString());
            }
        }else
        // PZArtBackground
        if ( className->isEqual( __String::create( C_ART_BACKGROUNDPLAY_CLASS_NAME ) ) ) {
            PZGArtBackgroundPlay *artObject = PZGArtBackgroundPlay::createWithDictionary(dict);
            __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
            if ( arr ){
                ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
            }
            else{
                arr = __Array::create( );
                arr->addObject( artObject );
                artResource->setObject(arr, artObject->key->getCString());
            }
        }else
        // PZArtCharacter
        if ( className->isEqual( __String::create( C_ART_CHARACTER_CLASS_NAME ) ) ){
            PZGArtCharacter *artObject = PZGArtCharacter::createWithDictionary(dict);
            __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
            if ( arr ){
                ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
            }
            else{
                arr = __Array::create( );
                arr->addObject( artObject );
                artResource->setObject(arr, artObject->key->getCString());
            }
        }else
        // PZArtObstacle
        if ( className->isEqual( __String::create( C_ART_OBSTACLE_CLASS_NAME ) ) ){
            PZGArtObstacle *artObject = PZGArtObstacle::createWithDictionary(dict);
            __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
            if ( arr ){
                ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
            }
            else{
                arr = __Array::create( );
                arr->addObject( artObject );
                artResource->setObject(arr, artObject->key->getCString());
            }
        }else        
        // PZArtCoins
        if ( className->isEqual( __String::create( C_ART_COINS_CLASS_NAME ) ) ){
            PZGArtCoins *artObject = PZGArtCoins::createWithDictionary(dict);
            __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
            if ( arr ){
                ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
            }
            else{
                arr = __Array::create( );
                arr->addObject( artObject );
                artResource->setObject(arr, artObject->key->getCString());
            }
        }else
        // PZArtInterface
        if ( className->isEqual( __String::create( C_ART_INTERFACE_CLASS_NAME ) ) ){
            PZGArtInterface *artObject = PZGArtInterface::createWithDictionary(dict);
            __Array* arr = ( __Array* )artResource->objectForKey( artObject->key->getCString() );
            if ( arr ){
                ((__Array*)artResource->objectForKey( artObject->key->getCString() ))->addObject( artObject );
            }
            else{
                arr = __Array::create( );
                arr->addObject( artObject );
                artResource->setObject(arr, artObject->key->getCString());
            }
        }else
        // PZSoundObject
        if ( className->isEqual( __String::create( C_SOUND_CLASS_NAME ) ) ){
            PZGSoundData *object = PZGSoundData::createWithDictionary(dict);
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
        }else
        // PZGameplayBasicRunner
        if ( className->isEqual( __String::create( C_GAMEPLAY_BASICRUNNER_CLASS_NAME ) ) ){
            PZGGameplayBasicRunner *obj = PZGGameplayBasicRunner::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplayBasicAvoidance
        if ( className->isEqual( __String::create( C_GAMEPLAY_BASICAVOIDANCE_CLASS_NAME ) ) ){
            PZGGameplayBasicAvoidance *obj = PZGGameplayBasicAvoidance::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplayHelicopterGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_HELICOPTER_CLASS_NAME ) ) ){
            PZGGameplayHelicopterGame *obj = PZGGameplayHelicopterGame::createWithDictionary(dict);          
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplaySpaceShooter
        if ( className->isEqual( __String::create( C_GAMEPLAY_SPACESHOOTER_CLASS_NAME ) ) ){
            PZGGameplaySpaceShooter *obj = PZGGameplaySpaceShooter::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplayShooterHelicopter
        if ( className->isEqual( __String::create( C_GAMEPLAY_SHOOTINGHELICOPTER_CLASS_NAME ) ) ){
            PZGGameplayShootingHelicopter *obj = PZGGameplayShootingHelicopter::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplayShootingRunner
        if ( className->isEqual( __String::create( C_GAMEPLAY_SHOOTINGRUNNER_CLASS_NAME ) ) ){
            PZGGameplayShootingRunner *obj = PZGGameplayShootingRunner::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplayTiltAvoidance
        if ( className->isEqual( __String::create( C_GAMEPLAY_TILTAVOIDANCE_CLASS_NAME ) ) ){
            PZGGameplayTiltAvoidance *obj = PZGGameplayTiltAvoidance::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplayRoundShooter
        if ( className->isEqual( __String::create( C_GAMEPLAY_ROUNDSHOOTER_CLASS_NAME ) ) ){
            PZGGameplayRoundShooter *obj = PZGGameplayRoundShooter::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplaySlowMoveRunner
        if ( className->isEqual( __String::create( C_GAMEPLAY_SLOWMOVERUNNER_CLASS_NAME ) ) ){
            PZGGameplaySlowMoveRunner *obj = PZGGameplaySlowMoveRunner::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameplayJumpingGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_JUMPING_CLASS_NAME ) ) ){
            PZGGameplayJumpingGame *obj = PZGGameplayJumpingGame::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGGameplayMegaJumpGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_MEGAJUMP_CLASS_NAME ) ) ){
            PZGGameplayMegaJumpGame *obj = PZGGameplayMegaJumpGame::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGGameplayRacingGame
        if ( className->isEqual( __String::create( C_GAMEPLAY_RACINGGAME_CLASS_NAME ) ) ){
            PZGGameplayRacingGame *obj = PZGGameplayRacingGame::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
           }
        }else
        // PZGameInfoGeneral
        if ( className->isEqual( __String::create( C_GAMEINFO_GENERAL_CLASS_NAME ) ) ){
            PZGGameInfoGeneral *obj = PZGGameInfoGeneral::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameInfoIAP
        if ( className->isEqual( __String::create( C_GAMEINFO_IAP_CLASS_NAME ) ) ){
            PZGGameInfoIAP *obj = PZGGameInfoIAP::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }else
        // PZGameInfoLevel
        if ( className->isEqual( __String::create( C_GAMEINFO_LEVEL_CLASS_NAME ) ) ){
            PZGGameInfoLevel *obj = PZGGameInfoLevel::createWithDictionary(dict);
            __Array* arr = ( __Array* )gameInfoResource->objectForKey( obj->key->getCString() );
            if ( arr ){
                ((__Array*)gameInfoResource->objectForKey( obj->key->getCString() ))->addObject( obj );
            }
            else{
                arr = __Array::create( );
                arr->addObject( obj );
                gameInfoResource->setObject(arr, obj->key->getCString());
            }
        }
    }
    
    artResource->retain();
    gameInfoResource->retain();
    soundResource->retain();
    
    synchronized = true;
}


