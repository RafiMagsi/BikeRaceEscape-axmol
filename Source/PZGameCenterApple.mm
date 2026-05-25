#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <GameKit/GameKit.h>

#include "PZGameCenter.h"
#include "cocos2dx_StoreController.h"

@interface PZGameCenterDelegateProxy : NSObject <GKGameCenterControllerDelegate>
@end

@implementation PZGameCenterDelegateProxy
- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController*)gameCenterViewController {
    [gameCenterViewController dismissViewControllerAnimated:YES completion:nil];
}
@end

namespace {

static NSString* g_leaderboardId = nil;
static UIActivityIndicatorView* g_spinner = nil;
static UIView* g_spinnerBackdrop = nil;
static PZGameCenterDelegateProxy* g_gcDelegateProxy = nil;

static UIViewController* _topMostViewController() {
    UIViewController* root = nil;

    if (@available(iOS 13.0, *)) {
        for (UIScene* scene in [UIApplication sharedApplication].connectedScenes) {
            if (scene.activationState != UISceneActivationStateForegroundActive) continue;
            if (![scene isKindOfClass:[UIWindowScene class]]) continue;
            UIWindowScene* windowScene = (UIWindowScene*)scene;
            for (UIWindow* window in windowScene.windows) {
                if (window.isKeyWindow) {
                    root = window.rootViewController;
                    break;
                }
            }
            if (root) break;
        }
    }

    if (!root) {
        root = [UIApplication sharedApplication].keyWindow.rootViewController;
    }
    if (!root) return nil;

    UIViewController* top = root;
    while (top.presentedViewController) {
        top = top.presentedViewController;
    }
    return top;
}

static UIWindow* _keyWindow() {
    if (@available(iOS 13.0, *)) {
        for (UIScene* scene in [UIApplication sharedApplication].connectedScenes) {
            if (scene.activationState != UISceneActivationStateForegroundActive) continue;
            if (![scene isKindOfClass:[UIWindowScene class]]) continue;
            UIWindowScene* windowScene = (UIWindowScene*)scene;
            for (UIWindow* window in windowScene.windows) {
                if (window.isKeyWindow) return window;
            }
        }
    }
    return [UIApplication sharedApplication].keyWindow;
}

static void _presentAlert(NSString* title, NSString* message) {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController* vc = _topMostViewController();
        if (!vc) return;

        UIAlertController* alert =
            [UIAlertController alertControllerWithTitle:title
                                                message:message
                                         preferredStyle:UIAlertControllerStyleAlert];
        [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];
        [vc presentViewController:alert animated:YES completion:nil];
    });
}

static NSString* _getLeaderboardId() {
    if (g_leaderboardId.length > 0) return g_leaderboardId;
    // Optional Info.plist override.
    id v = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"PZGameCenterLeaderboardId"];
    if ([v isKindOfClass:[NSString class]] && [(NSString*)v length] > 0) {
        return (NSString*)v;
    }
    return nil;
}

static void _ensureSpinner() {
    if (g_spinner) return;
    UIWindow* window = _keyWindow();
    if (!window) return;

    CGRect bounds = window.bounds;
    g_spinnerBackdrop = [[UIView alloc] initWithFrame:bounds];
    g_spinnerBackdrop.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent:0.35];
    g_spinnerBackdrop.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

    g_spinner = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleLarge];
    g_spinner.center = CGPointMake(CGRectGetMidX(bounds), CGRectGetMidY(bounds));
    g_spinner.autoresizingMask =
        UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin |
        UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin;

    [g_spinnerBackdrop addSubview:g_spinner];
}

} // namespace

namespace PZ {

GameCenter* GameCenter::shared() {
    static GameCenter instance;
    return &instance;
}

void GameCenter::setLeaderboardId(const char* leaderboardId) {
    if (!leaderboardId || leaderboardId[0] == '\0') {
        g_leaderboardId = nil;
        return;
    }
    g_leaderboardId = [NSString stringWithUTF8String:leaderboardId];
}

void GameCenter::playerLogin() {
    dispatch_async(dispatch_get_main_queue(), ^{
        GKLocalPlayer* localPlayer = GKLocalPlayer.localPlayer;
        localPlayer.authenticateHandler = ^(UIViewController* _Nullable viewController, NSError* _Nullable error) {
            if (viewController) {
                UIViewController* vc = _topMostViewController();
                if (vc) {
                    [vc presentViewController:viewController animated:YES completion:nil];
                }
                return;
            }

            if (error) {
                isEnabled = false;
                _presentAlert(@"Game Center", error.localizedDescription ?: @"Authentication failed.");
                return;
            }

            isEnabled = localPlayer.isAuthenticated;
        };
    });
}

void GameCenter::showLeaderboard() {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!GKLocalPlayer.localPlayer.isAuthenticated) {
            playerLogin();
            return;
        }

        UIViewController* vc = _topMostViewController();
        if (!vc) return;

        GKGameCenterViewController* gc = [[GKGameCenterViewController alloc] init];
        if (!gc) return;

        gc.viewState = GKGameCenterViewControllerStateLeaderboards;
        NSString* lb = _getLeaderboardId();
        if (lb.length > 0) {
            gc.leaderboardIdentifier = lb;
        }
        if (!g_gcDelegateProxy) {
            g_gcDelegateProxy = [PZGameCenterDelegateProxy new];
        }
        gc.gameCenterDelegate = (id<GKGameCenterControllerDelegate>)g_gcDelegateProxy;

        [vc presentViewController:gc animated:YES completion:nil];
    });
}

void GameCenter::restorePurchases() {
    // Legacy hook: route to our StoreKit bridge.
    cocos2dx_StoreController::restorePurchases();
}

void GameCenter::submitScore(int score) {
    if (score < 0) score = 0;

    dispatch_async(dispatch_get_main_queue(), ^{
        if (!GKLocalPlayer.localPlayer.isAuthenticated) {
            return;
        }

        if (@available(iOS 14.0, *)) {
            NSString* lb = _getLeaderboardId();
            NSArray<NSString*>* ids = lb.length > 0 ? @[lb] : @[];
            [GKLeaderboard submitScore:score
                               context:0
                                player:GKLocalPlayer.localPlayer
                        leaderboardIDs:ids
                     completionHandler:^(NSError* _Nullable error) {
                         if (error) {
                             // Non-fatal: don't spam UI during gameplay; just keep a lightweight alert path for debugging.
                         }
                     }];
        } else {
            NSString* lb = _getLeaderboardId();
            if (lb.length == 0) return;
            GKScore* gkScore = [[GKScore alloc] initWithLeaderboardIdentifier:lb];
            gkScore.value = score;
            [GKScore reportScores:@[gkScore] withCompletionHandler:nil];
        }
    });
}

void GameCenter::showProcessingView() {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIWindow* window = _keyWindow();
        if (!window) return;
        _ensureSpinner();
        if (!g_spinner || !g_spinnerBackdrop) return;
        if (!g_spinnerBackdrop.superview) {
            [window addSubview:g_spinnerBackdrop];
        }
        [g_spinner startAnimating];
    });
}

void GameCenter::hideProcessingView() {
    dispatch_async(dispatch_get_main_queue(), ^{
        [g_spinner stopAnimating];
        [g_spinnerBackdrop removeFromSuperview];
    });
}

void GameCenter::showErrorView(const char* message) {
    NSString* msg = message ? [NSString stringWithUTF8String:message] : @"Unknown error.";
    _presentAlert(@"Store Error", msg);
}

} // namespace PZ

#endif // iOS
