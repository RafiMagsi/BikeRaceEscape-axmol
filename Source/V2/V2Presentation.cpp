#include "V2/V2Presentation.h"
#include "PZLegacyCompat.h"

USING_NS_AX;

namespace V2 {
namespace Presentation {

static const char* kSpecKey = "PZP_V2_spectacle";
static const char* kMotionKey = "PZP_V2_reduced_motion";

Spectacle spectacle() {
    const int v = UserDefault::getInstance()->getIntegerForKey(kSpecKey, 0);
    return v == 1 ? Spectacle::Reduced : (v == 2 ? Spectacle::Off : Spectacle::Full);
}

void setSpectacle(Spectacle s) {
    UserDefault::getInstance()->setIntegerForKey(kSpecKey, static_cast<int>(s));
    UserDefault::getInstance()->flush();
}

Spectacle nextSpectacle(Spectacle s) {
    return s == Spectacle::Full ? Spectacle::Reduced : (s == Spectacle::Reduced ? Spectacle::Off : Spectacle::Full);
}

const char* spectacleName(Spectacle s) {
    return s == Spectacle::Full ? "FULL" : (s == Spectacle::Reduced ? "REDUCED" : "OFF");
}

bool reducedMotion() { return UserDefault::getInstance()->getBoolForKey(kMotionKey, false) || systemReduceMotion(); }

#if !defined(__APPLE__)
bool systemReduceMotion() { return false; }   // Android: AccessibilityManager / animator scale is not read yet
#endif

void setReducedMotion(bool on) {
    UserDefault::getInstance()->setBoolForKey(kMotionKey, on);
    UserDefault::getInstance()->flush();
}

} // namespace Presentation
} // namespace V2
