//
//  PZGameCenter.h
//  PZPlayer
//
//  Created by Nik Rudenko on 8/3/12.
//
//

#ifndef _PZ_GAME_CENTER_H_
#define _PZ_GAME_CENTER_H_

#include "Export.h"
#include <stddef.h>

namespace PZ {
    
class EXPORT_DLL GameCenter
{

    
public:
    GameCenter();
    ~GameCenter();
    
    static GameCenter* shared();
    static bool isAvialable();
    static void playerLogin();
    static void showLeaderboard();
    static void submitScore(int score);
    static void upgrade();
    static void restorePurchases();

    
    static void showProcessingView();
    static void hideProcessingView();
    static void showErrorView( const char* title );
    
    bool isEnabled;
};

}


#endif // _PZ_GAME_CENTER_H_