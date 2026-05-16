//
//  AdsManager.m
//  BikeRace
//
//  Created by Muhammad Rafi on 16/01/2015.
//
//

#import <UIKit/UIKit.h>
#import "AppController.h"
#import "AdsManager.h"
#import "cocos2d.h"

#import "PlayHavenSDK.h"

// Ads
//#import <Chartboost/Chartboost.h>
//#import <Chartboost/CBNewsfeed.h>

//#import <RevMobAds/RevMobAds.h>

#import "ALSdk.h"
#import "ALInterstitialAd.h"

@implementation AdsManager


-(void)ChartboostbootNow{
//    [Chartboost showInterstitial:CBLocationDefault];
    NSLog(@"Showing Chartboos\n");
}
-(void)ShowRevMobNow{
    
//    if(RevMobSessionStarted)
//    [[RevMobAds session] showFullscreen];
}

-(void)ShowRevMobPopUp{
//    if(RevMobSessionStarted)
//    [[RevMobAds session] showPopup];
}

-(void)ShowRevMobBanner{
//    if(RevMobSessionStarted)
//    [[RevMobAds session] showBanner];
}

-(void)RemoveRevMobBanner{
//    if(RevMobSessionStarted)
//    [[RevMobAds session] hideBanner];
}

-(void)ShowAppLovinNow{
    
    AppController *apC = (AppController*)[[UIApplication sharedApplication] delegate];
    [apC ShowAppLovinNow];
}


-(void) PHGameLaunch{
    AppController *apC = (AppController*)[[UIApplication sharedApplication] delegate];
    NSString *placement = @"game_launch";
    
    PHPublisherContentRequest *request = [PHPublisherContentRequest requestForApp:apC.token
                                                                           secret:apC.secret
                                                                        placement:placement
                                                                         delegate:apC];
    [request setShowsOverlayImmediately:YES];
    [request setAnimated:YES];
    [request send];
}

@end