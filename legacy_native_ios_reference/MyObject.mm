//
//  MyObject.mm
//  DogAgility
//
//  Created by Developer on 04/07/2013.
//
//

#import "MyObject.h"
#import "AdsManager.h"
#import "AppController.h"

@implementation MyObject

// C "trampoline" function to invoke Objective-C method
void MyObjectDoSomethingWith ()
{
    // Call the Objective-C method using Objective-C syntax
//    AppController *app  = (AppController*)[[UIApplication sharedApplication] delegate];
//    [app PHGameLaunch];
}

void ShowChartboost(){
    AdsManager *adM = [[AdsManager alloc] init];
    [adM ChartboostbootNow];
    
}

void ShowRevMob_C(){
    AdsManager *adM = [[AdsManager alloc] init];
    [adM ShowRevMobNow];
    
}

void ShowRevMobBanner_C(){
    AdsManager *adM = [[AdsManager alloc] init];
    [adM ShowRevMobBanner];
}

void ShowRevMobPopUp_C(){
    
    AdsManager *adM = [[AdsManager alloc] init];
    [adM ShowRevMobPopUp];
}
void HideRevMobBanner_C(){
    
    AdsManager *adM = [[AdsManager alloc] init];
    [adM RemoveRevMobBanner];
}

void ShowPlayHeaven_C(){
    AdsManager *adM = [[AdsManager alloc] init];
    [adM PHGameLaunch];
    
}

void ShowAppLovin_C(){
    
    AdsManager *adM = [[AdsManager alloc] init];
    [adM ShowAppLovinNow];
    
}
- (void) doSomethingWith
{
    // The Objective-C function you wanted to call from C++.
    // do work here..
//    return 21 ; // half of 42
}
@end