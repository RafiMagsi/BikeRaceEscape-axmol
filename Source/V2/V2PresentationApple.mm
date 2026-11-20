#include "V2/V2Presentation.h"
#import <TargetConditionals.h>
#if TARGET_OS_IOS
#import <UIKit/UIKit.h>
#endif

namespace V2 {
namespace Presentation {

bool systemReduceMotion() {
#if TARGET_OS_IOS
    return UIAccessibilityIsReduceMotionEnabled();
#else
    return false;
#endif
}

} // namespace Presentation
} // namespace V2
