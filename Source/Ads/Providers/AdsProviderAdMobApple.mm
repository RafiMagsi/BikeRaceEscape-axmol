#include "Ads/Providers/AdsProviderAdMob.h"

#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#include <atomic>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <dispatch/dispatch.h>

#if __has_include(<GoogleMobileAds/GoogleMobileAds.h>)
#import <GoogleMobileAds/GoogleMobileAds.h>
#define PZ_HAS_GMA 1
#else
#define PZ_HAS_GMA 0
#endif

#if PZ_HAS_GMA

#if !__has_feature(objc_arc)
#error "AdsProviderAdMobApple.mm must be compiled with -fobjc-arc (see CMakeLists.txt); without it loaded ads are never retained and presenting one crashes."
#endif

// All Google Mobile Ads state below is touched on the main queue only, except the two atomic "ready"
// flags, which the game code polls. Nothing here is reached through the C++ provider object, so the
// async blocks never depend on `this` staying alive.
//
// Lifetime rule (the cause of the show-time crash): an ad that has been handed to
// presentFromRootViewController: must stay strongly referenced until adDidDismissFullScreenContent:,
// and `fullScreenContentDelegate` is a *weak* property, so the delegate needs its own strong owner.
static GADBannerView* g_banner = nil;

static GADInterstitialAd* g_interstitial = nil;          // loaded, not shown yet
static GADInterstitialAd* g_presentingInterstitial = nil; // on screen, released on dismissal
static BOOL g_interstitialLoading = NO;
static std::atomic<bool> g_interstitialReady{false};

static GADRewardedAd* g_rewarded = nil;
static GADRewardedAd* g_presentingRewarded = nil;
static BOOL g_rewardedLoading = NO;
static std::atomic<bool> g_rewardedReady{false};

@interface PZAdMobFullScreenDelegate : NSObject <GADFullScreenContentDelegate>
@end

static PZAdMobFullScreenDelegate* g_fullScreenDelegate = nil;

// Logs banner results; GADBannerView keeps its delegate weakly, so g_bannerDelegate owns it.
@interface PZAdMobBannerDelegate : NSObject <GADBannerViewDelegate>
@end

@implementation PZAdMobBannerDelegate
- (void)bannerViewDidReceiveAd:(GADBannerView*)bannerView {
    AXLOGI("AdsProviderAdMob(iOS): banner loaded ({}x{})", (int)bannerView.bounds.size.width, (int)bannerView.bounds.size.height);
}
- (void)bannerView:(GADBannerView*)bannerView didFailToReceiveAdWithError:(NSError*)error {
    AXLOGW("AdsProviderAdMob(iOS): banner load failed: {}", error.localizedDescription.UTF8String ?: "");
}
@end

static PZAdMobBannerDelegate* g_bannerDelegate = nil;

@implementation PZAdMobFullScreenDelegate

- (void)adWillPresentFullScreenContent:(id<GADFullScreenPresentingAd>)ad {
    ax::AudioEngine::pauseAll();
}

- (void)ad:(id<GADFullScreenPresentingAd>)ad didFailToPresentFullScreenContentWithError:(NSError*)error {
    AXLOGW("AdsProviderAdMob(iOS): ad failed to present: {}", error.localizedDescription.UTF8String ?: "");
    ax::AudioEngine::resumeAll();
    if (ad == (id)g_presentingInterstitial) g_presentingInterstitial = nil;
    if (ad == (id)g_presentingRewarded) g_presentingRewarded = nil;
}

- (void)adDidDismissFullScreenContent:(id<GADFullScreenPresentingAd>)ad {
    AXLOGI("AdsProviderAdMob(iOS): full screen ad dismissed");
    ax::AudioEngine::resumeAll();
    if (ad == (id)g_presentingInterstitial) g_presentingInterstitial = nil;
    if (ad == (id)g_presentingRewarded) g_presentingRewarded = nil;
}

@end

static PZAdMobFullScreenDelegate* _fullScreenDelegate() {
    if (!g_fullScreenDelegate) g_fullScreenDelegate = [[PZAdMobFullScreenDelegate alloc] init];
    return g_fullScreenDelegate;
}

#endif // PZ_HAS_GMA

namespace PZ {

namespace {

#if PZ_HAS_GMA
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
    while (top.presentedViewController && !top.presentedViewController.isBeingDismissed) {
        top = top.presentedViewController;
    }
    return top;
}

static NSString* _nsString(const std::string& s) {
    return [NSString stringWithUTF8String:s.c_str()] ?: @"";
}
#endif

} // namespace

bool AdsProviderAdMob::initialize(const std::string& appIdOrKey) {
    (void)appIdOrKey;
    initialized_ = true;
    interstitialReady_ = false;
    rewardedReady_ = false;
    interstitialAutoShow_ = false;

#if PZ_HAS_GMA
    // Note: Modern GMA reads the App ID from Info.plist (GADApplicationIdentifier).
    // We keep `appIdOrKey` for compatibility with other providers / configs.
    // The app owns the audio session (see GameAppController): keep the SDK from switching its category,
    // which interrupts the menu music. The game's audio is paused/resumed around full-screen ads instead.
    [GADMobileAds sharedInstance].audioVideoManager.audioSessionIsApplicationManaged = YES;
    [[GADMobileAds sharedInstance] startWithCompletionHandler:nil];
    AXLOGI("AdsProviderAdMob(iOS): Google Mobile Ads started");

    // Set test device ID so we get test ads (not real ads)
    NSArray *testDeviceIDs = @[ @"be650e90ca1cb0f5e1cde5dbcf188455", @"19b2994fff154d01ff1a3869eaad7f63" ];
    [GADMobileAds sharedInstance].requestConfiguration.testDeviceIdentifiers = testDeviceIDs;
    AXLOGI("AdsProviderAdMob(iOS): Test device ID configured");

    return true;
#else
    AXLOGW("AdsProviderAdMob(iOS): Google Mobile Ads SDK not found; provider runs in stub mode (no ads will be shown).");
    return true;
#endif
}

void AdsProviderAdMob::loadBanner(const AdRequest& request) {
    if (!initialized_) return;
    lastBannerPlacement_ = request.placementId;
#if PZ_HAS_GMA
    AXLOGI("AdsProviderAdMob(iOS): banner placement set '{}'", lastBannerPlacement_);
#else
    AXLOGI("AdsProviderAdMob(iOS)::loadBanner (stub) placementId='{}'", lastBannerPlacement_);
#endif
}

void AdsProviderAdMob::showBanner() {
    if (!initialized_) return;
#if PZ_HAS_GMA
    if (lastBannerPlacement_.empty()) {
        AXLOGW("AdsProviderAdMob(iOS): showBanner called without placement id");
        return;
    }
    NSString* unitId = _nsString(lastBannerPlacement_);
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController* vc = _topMostViewController();
        if (!vc || !vc.view) {
            AXLOGW("AdsProviderAdMob(iOS): no view controller to present banner");
            return;
        }

        if (!g_banner) {
            // Use adaptive anchored banner size.
            const CGFloat width = vc.view.bounds.size.width;
            const GADAdSize adSize = GADCurrentOrientationAnchoredAdaptiveBannerAdSizeWithWidth(width);
            g_banner = [[GADBannerView alloc] initWithAdSize:adSize];
            g_banner.rootViewController = vc;
            if (!g_bannerDelegate) g_bannerDelegate = [[PZAdMobBannerDelegate alloc] init];
            g_banner.delegate = g_bannerDelegate;
            g_banner.translatesAutoresizingMaskIntoConstraints = NO;
            [vc.view addSubview:g_banner];
            [NSLayoutConstraint activateConstraints:@[
                [g_banner.centerXAnchor constraintEqualToAnchor:vc.view.centerXAnchor],
                [g_banner.bottomAnchor constraintEqualToAnchor:vc.view.safeAreaLayoutGuide.bottomAnchor],
            ]];
        }

        g_banner.adUnitID = unitId;
        [g_banner loadRequest:[GADRequest request]];
        g_banner.hidden = NO;
    });
#else
    AXLOGI("AdsProviderAdMob(iOS)::showBanner (stub)");
#endif
}

void AdsProviderAdMob::hideBanner() {
#if PZ_HAS_GMA
    dispatch_async(dispatch_get_main_queue(), ^{
        if (g_banner) g_banner.hidden = YES;
    });
#else
    AXLOGI("AdsProviderAdMob(iOS)::hideBanner (stub)");
#endif
}

bool AdsProviderAdMob::isInterstitialReady() const {
#if PZ_HAS_GMA
    return initialized_ && g_interstitialReady.load();
#else
    return initialized_ && interstitialReady_;
#endif
}

void AdsProviderAdMob::loadInterstitial(const AdRequest& request) {
    if (!initialized_) return;
    lastInterstitialPlacement_ = request.placementId;
#if PZ_HAS_GMA
    if (lastInterstitialPlacement_.empty()) return;
    NSString* unitId = _nsString(lastInterstitialPlacement_);
    dispatch_async(dispatch_get_main_queue(), ^{
        // Keep an already cached ad, and never start a second request while one is in flight.
        if (g_interstitial || g_interstitialLoading) return;
        g_interstitialLoading = YES;
        [GADInterstitialAd loadWithAdUnitID:unitId
                                    request:[GADRequest request]
                          completionHandler:^(GADInterstitialAd* ad, NSError* error) {
                              g_interstitialLoading = NO;
                              if (error || !ad) {
                                  AXLOGW("AdsProviderAdMob(iOS): interstitial load failed: {}", error.localizedDescription.UTF8String ?: "no ad");
                                  g_interstitialReady = false;
                                  return;
                              }
                              ad.fullScreenContentDelegate = _fullScreenDelegate();
                              g_interstitial = ad;
                              g_interstitialReady = true;
                              AXLOGI("AdsProviderAdMob(iOS): interstitial loaded");
                          }];
    });
#else
    interstitialReady_ = !lastInterstitialPlacement_.empty();
    AXLOGI("AdsProviderAdMob(iOS)::loadInterstitial (stub) placementId='{}'", lastInterstitialPlacement_);
#endif
}

void AdsProviderAdMob::showInterstitial() {
    if (!initialized_) return;
#if PZ_HAS_GMA
    // Only ever show a cached ad (an ad is never queued to pop up later, e.g. in the middle of a run).
    // Claim it right away so a second call in the same frame cannot present it twice.
    bool expected = true;
    if (!g_interstitialReady.compare_exchange_strong(expected, false)) {
        AXLOGI("AdsProviderAdMob(iOS): showInterstitial called but no interstitial is cached");
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        GADInterstitialAd* ad = g_interstitial;
        if (!ad) return;

        if (g_presentingInterstitial) {
            // An ad is already on screen: keep the cached one for later.
            g_interstitialReady = true;
            return;
        }

        UIViewController* vc = _topMostViewController();
        if (!vc) {
            AXLOGW("AdsProviderAdMob(iOS): cannot present interstitial (no view controller)");
            g_interstitialReady = true; // keep it, try again on the next tick
            return;
        }

        NSError* presentError = nil;
        if (![ad canPresentFromRootViewController:vc error:&presentError]) {
            // Expired or otherwise unusable: drop it so the next tick loads a fresh one.
            AXLOGW("AdsProviderAdMob(iOS): interstitial cannot be presented: {}", presentError.localizedDescription.UTF8String ?: "");
            g_interstitial = nil;
            return;
        }

        // Hand the ad over to g_presentingInterstitial; it is released in adDidDismissFullScreenContent:.
        g_interstitial = nil;
        g_presentingInterstitial = ad;
        ad.fullScreenContentDelegate = _fullScreenDelegate();
        [ad presentFromRootViewController:vc];
        AXLOGI("AdsProviderAdMob(iOS): interstitial presented");
    });
#else
    AXLOGI("AdsProviderAdMob(iOS)::showInterstitial (stub) placementId='{}'", lastInterstitialPlacement_);
    interstitialReady_ = false;
#endif
    interstitialAutoShow_ = false;
}

bool AdsProviderAdMob::isRewardedReady() const {
#if PZ_HAS_GMA
    return initialized_ && g_rewardedReady.load();
#else
    return initialized_ && rewardedReady_;
#endif
}

void AdsProviderAdMob::loadRewarded(const AdRequest& request) {
    if (!initialized_) return;
    lastRewardedPlacement_ = request.placementId;
#if PZ_HAS_GMA
    if (lastRewardedPlacement_.empty()) return;
    NSString* unitId = _nsString(lastRewardedPlacement_);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (g_rewarded || g_rewardedLoading) return;
        g_rewardedLoading = YES;
        [GADRewardedAd loadWithAdUnitID:unitId
                                request:[GADRequest request]
                      completionHandler:^(GADRewardedAd* ad, NSError* error) {
                          g_rewardedLoading = NO;
                          if (error || !ad) {
                              AXLOGW("AdsProviderAdMob(iOS): rewarded load failed: {}", error.localizedDescription.UTF8String ?: "no ad");
                              g_rewardedReady = false;
                              return;
                          }
                          ad.fullScreenContentDelegate = _fullScreenDelegate();
                          g_rewarded = ad;
                          g_rewardedReady = true;
                          AXLOGI("AdsProviderAdMob(iOS): rewarded loaded");
                      }];
    });
#else
    rewardedReady_ = !lastRewardedPlacement_.empty();
    AXLOGI("AdsProviderAdMob(iOS)::loadRewarded (stub) placementId='{}'", lastRewardedPlacement_);
#endif
}

void AdsProviderAdMob::showRewarded() {
    if (!initialized_) return;
#if PZ_HAS_GMA
    bool expected = true;
    if (!g_rewardedReady.compare_exchange_strong(expected, false)) {
        AXLOGI("AdsProviderAdMob(iOS)::showRewarded not ready");
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        GADRewardedAd* ad = g_rewarded;
        if (!ad) return;

        if (g_presentingRewarded || g_presentingInterstitial) {
            g_rewardedReady = true;
            return;
        }

        UIViewController* vc = _topMostViewController();
        if (!vc) {
            AXLOGW("AdsProviderAdMob(iOS): cannot present rewarded (no view controller)");
            g_rewardedReady = true;
            return;
        }

        NSError* presentError = nil;
        if (![ad canPresentFromRootViewController:vc error:&presentError]) {
            AXLOGW("AdsProviderAdMob(iOS): rewarded cannot be presented: {}", presentError.localizedDescription.UTF8String ?: "");
            g_rewarded = nil;
            return;
        }

        g_rewarded = nil;
        g_presentingRewarded = ad;
        ad.fullScreenContentDelegate = _fullScreenDelegate();
        [ad presentFromRootViewController:vc
                 userDidEarnRewardHandler:^{
                     GADAdReward* reward = ad.adReward;
                     AXLOGI("AdsProviderAdMob(iOS): earned reward type='{}' amount={}",
                            reward.type.UTF8String ?: "", reward.amount.intValue);
                 }];
    });
#else
    if (!rewardedReady_) {
        AXLOGI("AdsProviderAdMob(iOS)::showRewarded (stub) not ready");
        return;
    }
    AXLOGI("AdsProviderAdMob(iOS)::showRewarded (stub) placementId='{}'", lastRewardedPlacement_);
    rewardedReady_ = false;
#endif
}

} // namespace PZ

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
