#include "V2/V2Challenge.h"
#import <TargetConditionals.h>
#if TARGET_OS_IOS
#import <UIKit/UIKit.h>
#endif

namespace V2 {
namespace Challenge {

std::string clipboardText() {
#if TARGET_OS_IOS
    // iOS 16+ asks the player to allow pasting from other apps; only called after an explicit tap on the code button.
    NSString* s = [UIPasteboard generalPasteboard].string;
    return s ? std::string([s UTF8String]) : std::string();
#else
    return "";
#endif
}

} // namespace Challenge
} // namespace V2
