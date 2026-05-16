//
//  PZGameCenter.m
//  PZPlayer
//
//  Created by Nik Rudenko on 8/3/12.
//
//

#import "PZGameCenter.h"

#import "AppController.h"

#import "GameKit/GKLocalPlayer.h"
#import "GameKit/GKScore.h"
#import "GameKit/GKAchievement.h"

static void static_setEnable(bool enable){
    NSLog(@"enable GC %d", enable);
    
    PZ::GameCenter *gc = PZ::GameCenter::shared();
    gc->isEnabled = enable;
    
}



namespace PZ {
    static GameCenter *s_gc;
    UIAlertView *loadingView;
    
    GameCenter::GameCenter()
    {
        
    }
    
    GameCenter::~GameCenter()
    {
        
    }
    
    GameCenter* GameCenter::shared(){
        if (! s_gc) {
            s_gc = new GameCenter();
        }
        return s_gc;
    }

    bool GameCenter::isAvialable(){
        // Check for presence of GKLocalPlayer API.
        Class gcClass = (NSClassFromString(@"GKLocalPlayer"));
        
        // The device must be running running iOS 4.1 or later.
        NSString *reqSysVer = @"4.1";
        NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
        BOOL osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
        
        return (gcClass && osVersionSupported);
        
        return YES;
    }

    void GameCenter::playerLogin(){
        [[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError *error) {
            if (error == nil)
            {
                static_setEnable( true );
                
                NSLog(@" Authenticate local player complete");
                
            }
            else
            {
                static_setEnable( false );
                NSLog(@"Authenticate local player Error: %@", [error description]);
            }
        }];
    }
    
    
    void GameCenter::showLeaderboard(){
        AppController* app = (AppController*)[[UIApplication sharedApplication] delegate];
        [app showLeaderboard];
        
    }
    
    void GameCenter::submitScore(int score){
        [NSBundle mainBundle] ;
        
        NSString *category =[[[NSBundle mainBundle] bundleIdentifier] stringByAppendingString:@".leaderboard"];
        NSLog(@"Submitting score for identifier: %@", category);
        GKScore *scoreReporter = [[[GKScore alloc] initWithCategory:category] autorelease];
        scoreReporter.value = score;
        
        [scoreReporter reportScoreWithCompletionHandler:^(NSError *error) {
            if (error != nil)
            {
                NSLog(@"Submitting score error: %@", [error description]);
            }
            else {
                NSLog(@"Submitting score success");
            }
            
        }];
    }
    
    void GameCenter::restorePurchases(){
        AppController* app = (AppController*)[[UIApplication sharedApplication] delegate];
        [app restoreCompletedTransaction];
    }
    
    void GameCenter::upgrade(){
        AppController* app = (AppController*)[[UIApplication sharedApplication] delegate];
        [app upgrade];
    }
    
    void GameCenter::showProcessingView(){
        loadingView = [[UIAlertView alloc] initWithTitle:@"" message:@"" delegate:nil cancelButtonTitle:nil otherButtonTitles:nil];
        
        UIActivityIndicatorView *actInd = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
        actInd.frame = CGRectMake(128.0f, 45.0f, 25.0f, 25.0f);
        [loadingView addSubview:actInd];
        [actInd startAnimating];
        [actInd release];
        
        
        UILabel *l = [[UILabel alloc]init];
        l.frame = CGRectMake(100, -25, 210, 100);
        l.text = @"Please wait...";
        l.font = [UIFont fontWithName:@"Helvetica" size:16];
        l.textColor = [UIColor whiteColor];
        l.shadowColor = [UIColor blackColor];
        l.shadowOffset = CGSizeMake(1.0, 1.0);
        l.backgroundColor = [UIColor clearColor];
        [loadingView addSubview:l];
        [l release];
        
        [loadingView show];
    }
    
    void GameCenter::hideProcessingView(){
        if (loadingView) {
            [loadingView dismissWithClickedButtonIndex:0 animated:NO];
            loadingView = NULL;
        }
    }
    
    void GameCenter::showErrorView( const char* title ){
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Error"
                                                            message:[NSString stringWithCString: title encoding:NSStringEncodingConversionAllowLossy]
                                                           delegate:nil
                                                  cancelButtonTitle:@"Ok"
                                                  otherButtonTitles:nil];
        [alertView show];
    }

}