#include "Ads/Providers/AdsProviderChartboost.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <dispatch/dispatch.h>
#include <atomic>

#include "PZLegacyCompat.h"
#include "PZSettingsController.h"

#if __has_include(<ChartboostSDK/Chartboost.h>)
  #import <ChartboostSDK/Chartboost.h>
  #import <ChartboostSDK/CHBInterstitial.h>
  #define PZ_HAS_CHARTBOOST 1
#else
  #define PZ_HAS_CHARTBOOST 0
#endif

static UIViewController* PZTopMostViewController() {
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
    while (top.presentedViewController) top = top.presentedViewController;
    return top;
}

// ObjC declarations must be at global scope (not inside a C++ namespace).
#if PZ_HAS_CHARTBOOST
static CHBInterstitial* g_interstitial = nil;
static std::atomic<bool> g_interstitialCached{false};
static std::atomic<bool> g_chartboostStarted{false};
static std::atomic<bool> g_pendingCacheAfterStart{false};
static std::string g_pendingLocationAfterStart;

@interface PZChartboostInterstitialDelegateProxy : NSObject <CHBInterstitialDelegate>
@end

@implementation PZChartboostInterstitialDelegateProxy

- (void)didCacheAd:(CHBCacheEvent*)event error:(CHBCacheError*)error {
    if (error) {
        AXLOGW("AdsProviderChartboost(iOS): interstitial cache failed: {}",
               [[error localizedDescription] UTF8String]);
        g_interstitialCached.store(false);
        return;
    }
    g_interstitialCached.store(true);
    const std::string loc = g_pendingLocationAfterStart.empty() ? "Default" : g_pendingLocationAfterStart;
    AXLOGI("AdsProviderChartboost(iOS): interstitial cached (location='{}')", loc.c_str());
}

- (void)didShowAd:(CHBShowEvent*)event error:(CHBShowError*)error {
    (void)event;
    if (error) {
        AXLOGW("AdsProviderChartboost(iOS): interstitial show failed: {}",
               [[error localizedDescription] UTF8String]);
        return;
    }
    g_interstitialCached.store(false);
    AXLOGI("AdsProviderChartboost(iOS): interstitial shown");
}

- (void)didDismissAd:(CHBDismissEvent*)event {
    (void)event;
    AXLOGI("AdsProviderChartboost(iOS): interstitial dismissed");
}

@end

static PZChartboostInterstitialDelegateProxy* g_delegate = nil;
#endif

namespace PZ {

namespace {
static UIViewController* topMostViewController() { return PZTopMostViewController(); }
} // namespace

bool AdsProviderChartboost::initialize(const std::string& appIdOrKey) {
    initialized_ = true;
    interstitialReady_ = false;
    interstitialAutoShow_ = false;

#if !PZ_HAS_CHARTBOOST
    AXLOGW("AdsProviderChartboost(iOS): ChartboostSDK headers not found; pod not installed?");
    return true;
#else
    const auto sep = appIdOrKey.find('|');
    if (sep == std::string::npos) {
        AXLOGW("AdsProviderChartboost(iOS): initialize expects 'appId|appSignature' but got '{}'", appIdOrKey);
        return true;
    }
    appId_ = appIdOrKey.substr(0, sep);
    appSignature_ = appIdOrKey.substr(sep + 1);
    if (appId_.empty() || appSignature_.empty()) {
        AXLOGW("AdsProviderChartboost(iOS): initialize missing appId/appSignature");
        return true;
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        if (!g_delegate) g_delegate = [PZChartboostInterstitialDelegateProxy new];
        // Keep Chartboost logs quiet; the iOS console can get noisy (especially around WebKit assertions).
        // Raise to `CBLoggingLevelInfo` temporarily when debugging SDK integration.
        [Chartboost setLoggingLevel:CBLoggingLevelWarning];

        // COPPA (Child-directed) handling:
        // This game can be played by everyone, so we default to NOT child-directed.
        // If "Kid Mode" is enabled in settings, we treat the app as child-directed.
        bool childDirected = false;
        if (auto* sc = PZSettingsController::shared()) {
            childDirected = sc->kidMode;
        }
        [Chartboost addDataUseConsent:[CHBCOPPADataUseConsent isChildDirected:childDirected ? YES : NO]];
        AXLOGI("AdsProviderChartboost(iOS): COPPA isChildDirected={}", childDirected);

        [Chartboost startWithAppID:[NSString stringWithUTF8String:appId_.c_str()]
                      appSignature:[NSString stringWithUTF8String:appSignature_.c_str()]
                        completion:^(CHBStartError* error) {
                            if (error) {
                                AXLOGW("AdsProviderChartboost(iOS): start failed: {}", [[error localizedDescription] UTF8String]);
                            } else {
                                AXLOGI("AdsProviderChartboost(iOS): started");
                                g_chartboostStarted.store(true);

                                // If a cache was requested before start completed, perform it now.
                                if (g_pendingCacheAfterStart.exchange(false)) {
                                    const std::string loc = g_pendingLocationAfterStart.empty() ? "Default" : g_pendingLocationAfterStart;
                                    AXLOGI("AdsProviderChartboost(iOS): start completed; running pending cache (location='{}')", loc);

                                    if (!g_delegate) g_delegate = [PZChartboostInterstitialDelegateProxy new];
                                    g_interstitialCached.store(false);
                                    g_interstitial = [[CHBInterstitial alloc] initWithLocation:[NSString stringWithUTF8String:loc.c_str()]
                                                                                     delegate:g_delegate];
                                    [g_interstitial cache];
                                }
                            }
                        }];
    });
    return true;
#endif
}

void AdsProviderChartboost::loadBanner(const AdRequest&) {
    // Not used: project keeps AdMob banners.
}
void AdsProviderChartboost::showBanner() {}
void AdsProviderChartboost::hideBanner() {}

bool AdsProviderChartboost::isInterstitialReady() const {
    (void)interstitialReady_;
#if PZ_HAS_CHARTBOOST
    return initialized_ && g_interstitialCached.load();
#else
    return initialized_ && interstitialReady_;
#endif
}

void AdsProviderChartboost::loadInterstitial(const AdRequest& request) {
    if (!initialized_) return;

    lastInterstitialLocation_ = request.placementId.empty() ? "Default" : request.placementId;
    interstitialReady_ = false;

#if !PZ_HAS_CHARTBOOST
    AXLOGI("AdsProviderChartboost(iOS)::loadInterstitial (stub) location='{}'", lastInterstitialLocation_);
    return;
#else
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!g_chartboostStarted.load()) {
            // Start is async; remember request and run it right after start completes.
            g_pendingLocationAfterStart = lastInterstitialLocation_;
            g_pendingCacheAfterStart.store(true);
            AXLOGI("AdsProviderChartboost(iOS): cache requested before Chartboost started; queued (location='{}')",
                   lastInterstitialLocation_);
            return;
        }

        // If we already have a cached interstitial for this location, don't blow it away by recreating.
        // We'll show it first, then the caller can request another cache after presenting.
        if (g_interstitial && g_interstitialCached.load()) {
            AXLOGI("AdsProviderChartboost(iOS): loadInterstitial ignored (already cached)");
            return;
        }

        g_interstitialCached.store(false);
        if (!g_delegate) g_delegate = [PZChartboostInterstitialDelegateProxy new];

        // Recreate per-location; Chartboost interstitial objects are location-specific.
        g_pendingLocationAfterStart = lastInterstitialLocation_;
        g_interstitial = [[CHBInterstitial alloc] initWithLocation:[NSString stringWithUTF8String:lastInterstitialLocation_.c_str()]
                                                         delegate:g_delegate];
        [g_interstitial cache];
    });
#endif
}

void AdsProviderChartboost::showInterstitial() {
    if (!initialized_) return;

#if !PZ_HAS_CHARTBOOST
    AXLOGI("AdsProviderChartboost(iOS)::showInterstitial (stub)");
    return;
#else
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!g_interstitial) {
            AXLOGW("AdsProviderChartboost(iOS): showInterstitial called before loadInterstitial");
            return;
        }

        UIViewController* vc = topMostViewController();
        if (!vc) {
            AXLOGW("AdsProviderChartboost(iOS): cannot present interstitial (no view controller)");
            return;
        }

        // `isCached` is deprecated but still available; Chartboost recommends calling show which performs checks.
        // We'll use `isCached` as a fast readiness check and auto-cache+retry behavior.
        if ([g_interstitial respondsToSelector:@selector(isCached)] && !g_interstitial.isCached) {
            // Do NOT present until cached; presenting early causes a white loading screen.
            // Cache and return; the caller (AdsController) should try `showInterstitial()` later once ready.
            AXLOGI("AdsProviderChartboost(iOS): show requested but not cached; caching only (no auto-show)");
            [g_interstitial cache];
            return;
        }

        g_interstitialCached.store(false);
        [g_interstitial showFromViewController:vc];
    });
#endif
}

bool AdsProviderChartboost::isRewardedReady() const { return false; }
void AdsProviderChartboost::loadRewarded(const AdRequest&) {}
void AdsProviderChartboost::showRewarded() {}

} // namespace PZ

#endif // iOS
