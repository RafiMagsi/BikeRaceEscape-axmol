// iOS: shows a web page in an in-app Safari sheet (SFSafariViewController) so the player never leaves the game.
#include "PZLinks.h"
#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import <UIKit/UIKit.h>
#import <SafariServices/SafariServices.h>

namespace PZLinks {

void pzOpenInAppBrowser(const std::string& url) {
    NSString* s = [NSString stringWithUTF8String:url.c_str()];
    dispatch_async(dispatch_get_main_queue(), ^{
        NSURL* u = [NSURL URLWithString:s];
        if (!u) return;
        UIWindow* window = nil;
        for (UIScene* scene in UIApplication.sharedApplication.connectedScenes) {
            if ([scene isKindOfClass:[UIWindowScene class]]) {
                for (UIWindow* w in ((UIWindowScene*)scene).windows) { if (w.isKeyWindow) { window = w; break; } }
            }
            if (window) break;
        }
        if (!window) window = UIApplication.sharedApplication.windows.firstObject;
        UIViewController* vc = window.rootViewController;
        while (vc.presentedViewController) vc = vc.presentedViewController;
        if (!vc) { [UIApplication.sharedApplication openURL:u options:@{} completionHandler:nil]; return; }
        SFSafariViewController* safari = [[SFSafariViewController alloc] initWithURL:u];
        [vc presentViewController:safari animated:YES completion:nil];
    });
}

} // namespace PZLinks
#endif
