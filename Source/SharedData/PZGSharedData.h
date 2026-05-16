//
//  PZSharedData_cpp.h
//  ProjectZero
//
//  Created by Lion User on 26/09/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ProjectZero_PZGSharedData_h
#define ProjectZero_PZGSharedData_h
#include <string.h>
#include "PZLegacyCompat.h"

#include "PZGArtObject.h"
#include "PZGArtBackground.h"
#include "PZGArtBackgroundPlay.h"
#include "PZGArtCharacter.h"
#include "PZGArtObstacle.h"
#include "PZGArtCoins.h"
#include "PZGArtInterface.h"

#include "PZGSoundData.h"
#include "PZGFontObject.h"

#include "PZGGameInfoGeneral.h"
#include "PZGGameInfoLevel.h"
#include "PZGGameInfoIAP.h"

#include "PZGGameInfoObject.h"
#include "PZGGameplayBasicRunner.h"
#include "PZGGameplayBasicAvoidance.h"
#include "PZGGameplayHelicopterGame.h"
#include "PZGGameplaySpaceShooter.h"
#include "PZGGameplayShootingRunner.h"
#include "PZGGameplayShootingHelicopter.h"
#include "PZGGameplayTiltAvoidance.h"
#include "PZGGameplayRoundShooter.h"
#include "PZGGameplaySlowMoveRunner.h"
#include "PZGGameplayJumpingGame.h"
#include "PZGGameplayMegaJumpGame.h"
#include "PZGGameplayRacingGame.h"

class PZGSharedData : public ax::Ref
{
private:
    bool synchronized;
    
public:
    ax::__String * docName;
    ax::__String * documentURL;
    ax::__String * workingURL;
    
    ax::__Dictionary* artResource;
    ax::__Dictionary* soundResource;
    ax::__Dictionary* fontResource;
    ax::__Dictionary* gameInfoResource;
    
     
    PZGSharedData();
    ~PZGSharedData();
    static PZGSharedData* sharedInstanse();
    static void destroyInstanse();
    
    void readDataFromFile();
    
    void setDocumentURL(const char* url);
    void setWorkingURL(const char* url);
    ax::__String* getFullPath(const char* name);
    
//    ax::__String* getSoundResourcePath(ax::__String* key);
    
};


#endif
