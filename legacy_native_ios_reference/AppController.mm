/****************************************************************************
 Copyright (c) 2010 cocos2d-x.org
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#import <UIKit/UIKit.h>
#import "AppController.h"
#import "cocos2d.h"
#import "EAGLView.h"
#import "AppDelegate.h"



#import "RootViewController.h"

#import "GameKit/GKLocalPlayer.h"
#import "GameKit/GKScore.h"
#import "GameKit/GKAchievement.h"

#import "PZSettingsController.h"
#import "PZGameCenter.h"
#import "PZGSharedData.h"

#import "EventHandling.h"
#import "cocos2dx_EventHandlers.h"
#import "VirtualCurrencyPack.h"
#import "AppStoreItem.h"
#import "VirtualGood.h"
#import "VirtualCurrency.h"
#import "PlayHavenSDK.h"

// Ads
//#import <Chartboost/Chartboost.h>
//#import <Chartboost/CBNewsfeed.h>
//#import <RevMobAds/RevMobAds.h>

#import "ALSdk.h"
#import "ALInterstitialAd.h"

@implementation AppController
@synthesize viewController, token, secret;

#pragma mark -
#pragma mark Application lifecycle

// cocos2d application instance
static AppDelegate s_sharedApplication;
//void PHCall();


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    RevMobSessionStarted = false;
    
    
    // Applovin Initialize
    [ALSdk initializeSdk];
    
    // Starting RevMob Session
    [RevMobAds startSessionWithAppID:@"54b8180dcd82739209f3bdd5" andDelegate:self];
    
    // Initialize the Chartboost library
//    [Chartboost startWithAppId:@"54b811ef0d602532493517cc"
//                  appSignature:@"5b454c1b20bfa026b6c06d21baedae8b52327941"
//                      delegate:nil];
    
// Play Heaven Rafi
    
//    if (PH_BASE_URL == nil || [PH_BASE_URL isEqualToString:@""]) {
//        [defaults setValue:@"http://api2.playhaven.com" forKey:@"PHBaseUrl"];
//    }

    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    if (self.token == nil || [self.token isEqualToString:@""]) {
        [defaults setValue:@"dd4c2d0b605c455faf3afc44d7af84de" forKey:@"ExampleToken"];
    }
    
    if (self.secret == nil || [self.secret isEqualToString:@""]) {
        [defaults setValue:@"64cacadb025d4a4696052d65bfbcf28f" forKey:@"ExampleSecret"];
    }

    self.token    = [defaults valueForKey:@"ExampleToken"];
    self.secret   = [defaults valueForKey:@"ExampleSecret"];
    
    
    [[NSUserDefaults standardUserDefaults] synchronize];
    
    [PHPublisherOpenRequest requestForApp:token  secret:secret];
    // Play Heaven
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    gsd->readDataFromFile();
    
    CCArray *array = (CCArray*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral * gameInfoGeneral = (PZGGameInfoGeneral*)array->objectAtIndex(0);
    if(gameInfoGeneral->isGameCenterEnabled){
       PZ::GameCenter* gc = PZ::GameCenter::shared();
       if( gc->isAvialable() ){
//           gc->playerLogin();
       }
    }

    
    // Add the view controller's view to the window and display.
    window = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
    EAGLView *__glView = [EAGLView viewWithFrame: [window bounds]
										pixelFormat: kEAGLColorFormatRGBA8
										depthFormat: GL_DEPTH_COMPONENT16
								 preserveBackbuffer: NO
                                                                                 sharegroup:nil
                                                                          multiSampling:NO
                                                                    numberOfSamples:0];
    
    __glView.multipleTouchEnabled = YES;
    // Use RootViewController manage EAGLView 
    viewController = [[RootViewController alloc] initWithNibName:nil bundle:nil];
    viewController.wantsFullScreenLayout = YES;
    viewController.view = __glView;

    
    
    
    if ( [[UIDevice currentDevice].systemVersion floatValue] < 6.0) {
        [window addSubview: viewController.view];
    }
    else  {
        [window setRootViewController:viewController];
    }
    
    [window makeKeyAndVisible];
    
    
//    [self ChartboostbootNow];
//    [[UIApplication sharedApplication] setStatusBarHidden: YES];
    
    [EventHandling observeAllEventsWithObserver:self withSelector:@selector(eventFired:)];
    
    cocos2d::CCApplication::sharedApplication()->run();
    return YES;
}

- (void)eventFired:(NSNotification*)notification{
    if ([notification.name isEqualToString:EVENT_APPSTORE_PURCHASED]) {
        AppStoreItem* apItem = (AppStoreItem*)[notification.userInfo objectForKey:@"AppStoreItem"];
        string productId([apItem.productId UTF8String]);
        cocos2dx_EventHandlers::getInstance()->marketPurchase(productId);
    }
    else if ([notification.name isEqualToString:EVENT_VIRTUAL_GOOD_PURCHASED]) {
        VirtualGood* good = (VirtualGood*)[notification.userInfo objectForKey:@"VirtualGood"];
        string itemId([good.itemId UTF8String]);
        cocos2dx_EventHandlers::getInstance()->virtualGoodPurchased(itemId);
    }
    else if ([notification.name isEqualToString:EVENT_VIRTUAL_GOOD_EQUIPPED]) {
        VirtualGood* good = (VirtualGood*)[notification.userInfo objectForKey:@"VirtualGood"];
        string itemId([good.itemId UTF8String]);
        cocos2dx_EventHandlers::getInstance()->virtualGoodEquipped(itemId);
    }
    else if ([notification.name isEqualToString:EVENT_VIRTUAL_GOOD_UNEQUIPPED]) {
        VirtualGood* good = (VirtualGood*)[notification.userInfo objectForKey:@"VirtualGood"];
        string itemId([good.itemId UTF8String]);
        cocos2dx_EventHandlers::getInstance()->virtualGoodUnequipped(itemId);
    }
    else if ([notification.name isEqualToString:EVENT_BILLING_SUPPORTED]) {
        cocos2dx_EventHandlers::getInstance()->billingSupported();
    }
    else if ([notification.name isEqualToString:EVENT_BILLING_NOT_SUPPORTED]) {
        cocos2dx_EventHandlers::getInstance()->billingNotSupported();
    }
    else if ([notification.name isEqualToString:EVENT_MARKET_PURCHASE_STARTED]) {
        string productId("");
        cocos2dx_EventHandlers::getInstance()->marketPurchaseProcessStarted(productId);
    }
    else if ([notification.name isEqualToString:EVENT_GOODS_PURCHASE_STARTED]) {
        cocos2dx_EventHandlers::getInstance()->goodsPurchaseProcessStarted();
    }
    else if ([notification.name isEqualToString:EVENT_CLOSING_STORE]) {
        cocos2dx_EventHandlers::getInstance()->closingStore();
    }
    else if ([notification.name isEqualToString:EVENT_OPENING_STORE]) {
        cocos2dx_EventHandlers::getInstance()->openingStore();
    }
    else if ([notification.name isEqualToString:EVENT_UNEXPECTED_ERROR_IN_STORE]) {
        NSString *message = [notification.userInfo objectForKey:@"ErrorMessage"];
        string messageStr([message UTF8String]);
        cocos2dx_EventHandlers::getInstance()->unexpectedErrorInStore( messageStr );
    }
    else if ([notification.name isEqualToString:EVENT_PAYMENT_CANCELLED]) {
        cocos2dx_EventHandlers::getInstance()->paymentCancelled( );
    }
    else if ([notification.name isEqualToString:EVENT_CHANGED_CURRENCY_BALANCE]) {
		int balance = [(NSNumber*)[notification.userInfo objectForKey:@"balance"] intValue];
		VirtualCurrency* currency = (VirtualCurrency*)[notification.userInfo objectForKey:@"VirtualCurrency"];
		string itemId([currency.itemId UTF8String]);
        cocos2dx_EventHandlers::getInstance()->currencyBalanceChanged(itemId, balance);
    }
    else if ([notification.name isEqualToString:EVENT_CHANGED_GOOD_BALANCE]) {
		int balance = [(NSNumber*)[notification.userInfo objectForKey:@"balance"] intValue];
		VirtualGood* good = (VirtualGood*)[notification.userInfo objectForKey:@"VirtualGood"];
		string itemId([good.itemId UTF8String]);
        cocos2dx_EventHandlers::getInstance()->goodBalanceChanged(itemId, balance);
    }
    else if ([notification.name isEqualToString:EVENT_TRANSACTION_RESTORED]) {
        NSString* productIdString = [notification.userInfo objectForKey:@"RestoredProductId"];
        string productId([productIdString UTF8String]);
        cocos2dx_EventHandlers::getInstance()->transactionRestored(productId);
    }
}

-(NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        return UIInterfaceOrientationMaskAll;
    else  /* iphone */
        return UIInterfaceOrientationMaskAllButUpsideDown;
}

- (void)applicationWillResignActive:(UIApplication *)application {
	cocos2d::CCDirector::sharedDirector()->pause();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	cocos2d::CCDirector::sharedDirector()->resume();
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    cocos2d::CCApplication::sharedApplication()->applicationDidEnterBackground();
    [self startDailyNotifications];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    cocos2d::CCApplication::sharedApplication()->applicationWillEnterForeground();
    [self removeOldNotifications];
    [PHPublisherOpenRequest requestForApp:token  secret:secret];
}

- (void)applicationWillTerminate:(UIApplication *)application {

}


#pragma mark IAP
- (void)upgrade{
	if (![SKPaymentQueue canMakePayments]){
		UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:@"Error"
															message:@"In App purchase Disabled"
														   delegate:self
												  cancelButtonTitle:@"Ok"
												  otherButtonTitles:nil];
		[alertView show];
		[alertView release];
		
		return;
	}

	NSString *productId = [[[NSBundle mainBundle] bundleIdentifier] stringByAppendingString:@".upgrade"];
    
	SKPayment *payment;
	payment = [SKPayment paymentWithProductIdentifier:productId];
	
	[[SKPaymentQueue defaultQueue] addPayment:payment];
    
	loadingView = [[UIAlertView alloc] initWithTitle:@"" message:@"" delegate:self cancelButtonTitle:nil otherButtonTitles:nil];
	
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

- (void)restoreCompletedTransaction{
    [[SKPaymentQueue defaultQueue]restoreCompletedTransactions];
    
//    loadingView = [[UIAlertView alloc] initWithTitle:@"" message:@"" delegate:self cancelButtonTitle:nil otherButtonTitles:nil];
//	
//	UIActivityIndicatorView *actInd = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
//	actInd.frame = CGRectMake(128.0f, 45.0f, 25.0f, 25.0f);
//	[loadingView addSubview:actInd];
//	[actInd startAnimating];
//	[actInd release];
//	
//    
//	UILabel *l = [[UILabel alloc]init];
//	l.frame = CGRectMake(100, -25, 210, 100);
//	l.text = @"Please wait...";
//	l.font = [UIFont fontWithName:@"Helvetica" size:16];
//	l.textColor = [UIColor whiteColor];
//	l.shadowColor = [UIColor blackColor];
//	l.shadowOffset = CGSizeMake(1.0, 1.0);
//	l.backgroundColor = [UIColor clearColor];
//	[loadingView addSubview:l];
//	[l release];
//    
//	[loadingView show];
}


-(void)transactionDidFinishRestoring{
    
//    PZSettingsController *sc = PZSettingsController::shared();
//    sc->unlockAllPlayers();
//    [loadingView dismissWithClickedButtonIndex:0 animated:NO];
//
//    [viewController removeAd];
    
}

-(void)transactionDidFinish:(NSString*)transactionIdentifier{
    
//	[loadingView dismissWithClickedButtonIndex:0 animated:NO];
//	NSString *productId = [[[NSBundle mainBundle] bundleIdentifier] stringByAppendingString:@".upgrade"];
//    
//    if([transactionIdentifier isEqualToString:productId]){
//        [viewController removeAd];
//    }
//    
//    PZSettingsController *sc = PZSettingsController::shared();
//    sc->unlockAllPlayers();

}

-(void)transactionDidError{
	[loadingView dismissWithClickedButtonIndex:0 animated:NO];
}


#pragma mark GAME CENTER

- (void) showLeaderboard{
	NSLog(@"show leaderboard");
	
    GKLeaderboardViewController *leaderboardController = [[GKLeaderboardViewController alloc] init];
    if (leaderboardController != nil)
    {
        leaderboardController.leaderboardDelegate = self;
        [viewController presentModalViewController:leaderboardController animated: YES];
    }
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)_viewController{
	[_viewController dismissModalViewControllerAnimated:YES];
}

#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}

#pragma mark LocalNotifications

- (void) registerLocalNotificationWithAction:(NSString *)action
										Body:(NSString *)body
									FireTime:(NSTimeInterval)fireTime
								  Identifier:(int)ID
{
    /*
     NSTimeInterval idleTime = [[defprefs objectForKey:kDEFAULT_IDLE_TIME] floatValue];
     
     if (fireTime < idleTime && ID != kNOTIFY_TYPE_20HOUR)
     return;
     
     if ([defprefs objectForKey:[NSString stringWithFormat:@"notify-%d",ID]])
     return;
     
     NSDate *fireDate;
     if (ID == kNOTIFY_TYPE_20HOUR)
     fireDate = [NSDate dateWithTimeIntervalSinceNow:fireTime];
     else
     fireDate = [NSDate dateWithTimeIntervalSinceNow:fireTime - idleTime];
     */
    
    NSDate *fireDate = [NSDate dateWithTimeIntervalSinceNow:fireTime];
    
	UILocalNotification *notif = [[UILocalNotification alloc] init];
	if (notif)
	{
		notif.fireDate = fireDate;
		notif.timeZone = [NSTimeZone defaultTimeZone];
		notif.repeatInterval = 0;
        //		notif.soundName = UILocalNotificationDefaultSoundName;
        notif.soundName = @"ha2_ln.mp3";
		notif.alertBody = body;
		notif.alertAction = action;
        NSMutableDictionary *infoDict = [[NSMutableDictionary alloc] init];
        [infoDict setObject:[NSNumber numberWithInt:ID] forKey:@"notify-id"];
        [infoDict setObject:[NSNumber numberWithInt:fireTime] forKey:@"notify-fire-time"];
		notif.userInfo = infoDict;
		
		[[UIApplication sharedApplication] scheduleLocalNotification:notif];
		[notif release];
	}
}


- (BOOL) isNotificationEnabled {
    
    // The device must be running running iOS 4.1 or later.
    NSString *reqSysVer = @"4.0";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    BOOL osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
	
    return osVersionSupported;
}

- (void) startDailyNotifications
{
	
	if (![self isNotificationEnabled])
		return;
	
    
    // 1 day
//	[self registerLocalNotificationWithAction:@"Yeah"
//										 Body:@"Lets play Bike Race!"
//									 FireTime:86400 * 1
//								   Identifier:1];
    // 3 days
	[self registerLocalNotificationWithAction:@"Yeah"
										 Body:@"Time for a quick game? Your friends at Bike Race Road!"
									 FireTime:86400 * 3
								   Identifier:3];
    // 7 days
    [self registerLocalNotificationWithAction:@"Yeah"
										 Body:@"We've missed you! How about a game now?"
									 FireTime:86400 * 7
								   Identifier:7];
    // 21 days
//    [self registerLocalNotificationWithAction:@"Yeah"
//										 Body:@"Play Bike Race!"
//									 FireTime:86400 * 9
//								   Identifier:21];
}

- (void) removeOldNotifications
{
	if (![self isNotificationEnabled])
		return;
    
    [[UIApplication sharedApplication] cancelAllLocalNotifications];
    /*
     
     NSTimeInterval idleTime = [[defprefs objectForKey:kDEFAULT_IDLE_TIME] floatValue];
     NSTimeInterval nowTime = [NSDate timeIntervalSinceReferenceDate];
     NSTimeInterval idleStartTime = nowTime;
     if ([defprefs objectForKey:kDEFAULT_IDLE_START_TIME])
     {
     idleStartTime = [[defprefs objectForKey:kDEFAULT_IDLE_START_TIME] floatValue];
     }
     
     if (nowTime - idleStartTime > 24 * 3600)
     {
     NSArray *oldNotifs = [[UIApplication sharedApplication] scheduledLocalNotifications];
     if ([oldNotifs count] > 0)
     {
     [[UIApplication sharedApplication] cancelAllLocalNotifications];
     }
     idleTime = 0;
     }
     else
     {
     idleTime += (nowTime - idleStartTime);
     [defprefs setObject:[NSNumber numberWithFloat:idleTime] forKey:kDEFAULT_IDLE_TIME];
     [defprefs synchronize];
     
     NSArray *oldNotifs = [[UIApplication sharedApplication] scheduledLocalNotifications];
     
     for (UILocalNotification *notify in oldNotifs)
     {
     NSTimeInterval fireTime = [[notify.userInfo objectForKey:@"notify-fire-time"] floatValue];
     if (fireTime < idleTime)
     {
     [self processNotification:notify];
     }
     }
     [[UIApplication sharedApplication] cancelAllLocalNotifications];
     }
     */
	
}

-(void)ShowAppLovinNow{
    
    [ALInterstitialAd showOver:viewController.view.window];
}


-(void)revmobSessionIsStarted {
    NSLog(@"[RevMob Sample App] Session is started.");
    RevMobSessionStarted = true;
}
-(void)revmobAdDidFailWithError:(NSError *)error{
    NSLog(@"Rev Mob Ad Fail With Some Error");
}


- (void)dealloc {
    [super dealloc];
}


@end

