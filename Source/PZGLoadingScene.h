//
//  PZGLoadingScene.h
//  ProjectMayhem
//
//  Created by Nik Rudenko on 3/28/13.
//
//

#ifndef __ProjectMayhem__PZGLoadingScene__
#define __ProjectMayhem__PZGLoadingScene__

#include <iostream>
#include "PZLegacyCompat.h"


USING_NS_AX;

class PZGLoadingScene : public Layer
{
public:
    
	static ax::Scene* scene();
    virtual bool init();
  	
    LAYER_CREATE_FUNC( PZGLoadingScene )

    
    void continueLoadingSelector( float dt );
};


#endif /* defined(__ProjectMayhem__PZGLoadingScene__) */
