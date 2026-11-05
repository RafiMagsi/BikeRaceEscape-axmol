//
//  PZHapticsApple.mm
//  iOS haptics through UIKit feedback generators (Taptic Engine). Not ARC: generators live for the process.
//
#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <UIKit/UIKit.h>
#include "PZHaptics.h"

namespace {
UIImpactFeedbackGenerator* impact(UIImpactFeedbackStyle style) {
    static UIImpactFeedbackGenerator* gens[5] = {nil, nil, nil, nil, nil};
    const int i = static_cast<int>(style);
    if (!gens[i]) {
        gens[i] = [[UIImpactFeedbackGenerator alloc] initWithStyle:style];
    }
    return gens[i];
}
UINotificationFeedbackGenerator* notifier() {
    static UINotificationFeedbackGenerator* g = nil;
    if (!g) g = [[UINotificationFeedbackGenerator alloc] init];
    return g;
}
UISelectionFeedbackGenerator* selector() {
    static UISelectionFeedbackGenerator* g = nil;
    if (!g) g = [[UISelectionFeedbackGenerator alloc] init];
    return g;
}
void fire(UIImpactFeedbackStyle style, CGFloat intensity) {
    UIImpactFeedbackGenerator* g = impact(style);
    [g impactOccurredWithIntensity:intensity];
    [g prepare];
}
} // namespace

void PZHaptics_native(int kind) {
    switch (static_cast<PZHaptics::Kind>(kind)) {
    case PZHaptics::Kind::Tick:      fire(UIImpactFeedbackStyleSoft, 0.55); break;
    case PZHaptics::Kind::Light:     fire(UIImpactFeedbackStyleLight, 0.8); break;
    case PZHaptics::Kind::Medium:    fire(UIImpactFeedbackStyleMedium, 0.9); break;
    case PZHaptics::Kind::Heavy:     fire(UIImpactFeedbackStyleHeavy, 1.0); break;
    case PZHaptics::Kind::Success:   [notifier() notificationOccurred:UINotificationFeedbackTypeSuccess]; break;
    case PZHaptics::Kind::Warning:   [notifier() notificationOccurred:UINotificationFeedbackTypeWarning]; break;
    case PZHaptics::Kind::Error:     [notifier() notificationOccurred:UINotificationFeedbackTypeError]; break;
    case PZHaptics::Kind::Selection: [selector() selectionChanged]; break;
    }
}

#endif
