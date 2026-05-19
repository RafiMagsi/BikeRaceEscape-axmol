#include "Ads/Providers/AdsProviderAdMob.h"

#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <dispatch/dispatch.h>

#if __has_include(<GoogleMobileAds/GoogleMobileAds.h>)
#import <GoogleMobileAds/GoogleMobileAds.h>
#define PZ_HAS_GMA 1
#else
#define PZ_HAS_GMA 0
#endif

namespace PZ {

namespace {
#if PZ_HAS_GMA
static GADBannerView* g_banner = nil;
static GADInterstitialAd* g_interstitial = nil;
static GADRewardedAd* g_rewarded = nil;
#else
static id g_banner = nil;
static id g_interstitial = nil;
static id g_rewarded = nil;
#endif

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
} // namespace

bool AdsProviderAdMob::initialize(const std::string& appIdOrKey) {
    (void)appIdOrKey;
    initialized_ = true;
    interstitialReady_ = false;
    rewardedReady_ = false;

#if PZ_HAS_GMA
    // Note: Modern GMA reads the App ID from Info.plist (GADApplicationIdentifier).
    // We keep `appIdOrKey` for compatibility with other providers / configs.
    [[GADMobileAds sharedInstance] startWithCompletionHandler:nil];
    AXLOGI("AdsProviderAdMob(iOS): Google Mobile Ads started");
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
    dispatch_async(dispatch_get_main_queue(), ^{
        if (lastBannerPlacement_.empty()) {
            AXLOGW("AdsProviderAdMob(iOS): showBanner called without placement id");
            return;
        }

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
            g_banner.translatesAutoresizingMaskIntoConstraints = NO;
            [vc.view addSubview:g_banner];
            [NSLayoutConstraint activateConstraints:@[
                [g_banner.centerXAnchor constraintEqualToAnchor:vc.view.centerXAnchor],
                [g_banner.bottomAnchor constraintEqualToAnchor:vc.view.safeAreaLayoutGuide.bottomAnchor],
            ]];
        }

        g_banner.adUnitID = [NSString stringWithUTF8String:lastBannerPlacement_.c_str()];
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
    return initialized_ && interstitialReady_;
}

void AdsProviderAdMob::loadInterstitial(const AdRequest& request) {
    if (!initialized_) return;
    lastInterstitialPlacement_ = request.placementId;
#if PZ_HAS_GMA
    interstitialReady_ = false;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (lastInterstitialPlacement_.empty()) return;
        [GADInterstitialAd loadWithAdUnitID:[NSString stringWithUTF8String:lastInterstitialPlacement_.c_str()]
                                    request:[GADRequest request]
                          completionHandler:^(GADInterstitialAd* ad, NSError* error) {
                              if (error) {
                                  AXLOGW("AdsProviderAdMob(iOS): interstitial load failed: {}", [[error localizedDescription] UTF8String]);
                                  interstitialReady_ = false;
                                  return;
                              }
                              g_interstitial = ad;
                              interstitialReady_ = (g_interstitial != nil);
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
    if (!interstitialReady_) {
        AXLOGI("AdsProviderAdMob(iOS)::showInterstitial (stub) not ready");
        return;
    }
#if PZ_HAS_GMA
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController* vc = _topMostViewController();
        if (!vc) return;
        if (!g_interstitial) {
            AXLOGW("AdsProviderAdMob(iOS): interstitial not loaded");
            return;
        }
        [g_interstitial presentFromRootViewController:vc];
        g_interstitial = nil;
    });
#else
    AXLOGI("AdsProviderAdMob(iOS)::showInterstitial (stub) placementId='{}'", lastInterstitialPlacement_);
#endif
    interstitialReady_ = false;
}

bool AdsProviderAdMob::isRewardedReady() const {
    return initialized_ && rewardedReady_;
}

void AdsProviderAdMob::loadRewarded(const AdRequest& request) {
    if (!initialized_) return;
    lastRewardedPlacement_ = request.placementId;
#if PZ_HAS_GMA
    rewardedReady_ = false;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (lastRewardedPlacement_.empty()) return;
        [GADRewardedAd loadWithAdUnitID:[NSString stringWithUTF8String:lastRewardedPlacement_.c_str()]
                                request:[GADRequest request]
                      completionHandler:^(GADRewardedAd* ad, NSError* error) {
                          if (error) {
                              AXLOGW("AdsProviderAdMob(iOS): rewarded load failed: {}", [[error localizedDescription] UTF8String]);
                              rewardedReady_ = false;
                              return;
                          }
                          g_rewarded = ad;
                          rewardedReady_ = (g_rewarded != nil);
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
    if (!rewardedReady_) {
        AXLOGI("AdsProviderAdMob(iOS)::showRewarded (stub) not ready");
        return;
    }
#if PZ_HAS_GMA
    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController* vc = _topMostViewController();
        if (!vc) return;
        if (!g_rewarded) {
            AXLOGW("AdsProviderAdMob(iOS): rewarded not loaded");
            return;
        }
        [g_rewarded presentFromRootViewController:vc
                         userDidEarnRewardHandler:^{
                             GADAdReward* reward = g_rewarded.adReward;
                             AXLOGI("AdsProviderAdMob(iOS): earned reward type='{}' amount={}",
                                    [[reward type] UTF8String], [[reward amount] intValue]);
                         }];
        g_rewarded = nil;
    });
#else
    AXLOGI("AdsProviderAdMob(iOS)::showRewarded (stub) placementId='{}'", lastRewardedPlacement_);
#endif
    rewardedReady_ = false;
}

} // namespace PZ

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
