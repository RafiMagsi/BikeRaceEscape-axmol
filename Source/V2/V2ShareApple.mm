// iOS share sheet for the daily result card.
#include "V2/V2Share.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import <UIKit/UIKit.h>

namespace V2 {
namespace Share {

void sendImage(const std::string& path, const std::string& text) {
    NSString* p = [NSString stringWithUTF8String:path.c_str()];
    NSString* t = [NSString stringWithUTF8String:text.c_str()];
    dispatch_async(dispatch_get_main_queue(), ^{
        UIImage* img = [UIImage imageWithContentsOfFile:p];
        NSMutableArray* items = [NSMutableArray array];
        if (img) [items addObject:img];          // image first: the sheet previews it
        [items addObject:t];
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
        if (!vc) return;
        UIActivityViewController* sheet = [[UIActivityViewController alloc] initWithActivityItems:items applicationActivities:nil];
        if (sheet.popoverPresentationController) {              // iPad needs an anchor
            sheet.popoverPresentationController.sourceView = vc.view;
            sheet.popoverPresentationController.sourceRect = CGRectMake(CGRectGetMidX(vc.view.bounds), CGRectGetMidY(vc.view.bounds), 1, 1);
            sheet.popoverPresentationController.permittedArrowDirections = 0;
        }
        [vc presentViewController:sheet animated:YES completion:nil];
    });
}

} // namespace Share
} // namespace V2
#endif
