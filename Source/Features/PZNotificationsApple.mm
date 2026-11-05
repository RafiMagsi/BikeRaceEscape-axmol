//
//  PZNotificationsApple.mm
//  iOS local notifications via UNUserNotificationCenter. No push server, no entitlement: everything is scheduled on the
//  device. Not ARC (matches the rest of the Features sources).
//
#include "PZLegacyCompat.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#import <UserNotifications/UserNotifications.h>
#include "PZNotifications.h"

namespace {
int g_status = static_cast<int>(PZNotif::Status::Unknown);

int mapStatus(UNAuthorizationStatus s) {
    switch (s) {
    case UNAuthorizationStatusNotDetermined: return static_cast<int>(PZNotif::Status::NotDetermined);
    case UNAuthorizationStatusDenied:        return static_cast<int>(PZNotif::Status::Denied);
    default:                                 return static_cast<int>(PZNotif::Status::Granted);   // authorized / provisional / ephemeral
    }
}
} // namespace

int PZNotif_nativeStatus() {
    static bool first = true;
    if (first) {
        first = false;
        PZNotif_nativeRefresh();
    }
    return g_status;
}

void PZNotif_nativeRefresh() {
    [[UNUserNotificationCenter currentNotificationCenter] getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings* settings) {
        g_status = mapStatus(settings.authorizationStatus);
    }];
}

void PZNotif_nativeRequest() {
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge)
                          completionHandler:^(BOOL granted, NSError* error) {
        g_status = granted ? static_cast<int>(PZNotif::Status::Granted) : static_cast<int>(PZNotif::Status::Denied);
        const bool ok = granted;
        // Back onto the game thread before touching game state.
        dispatch_async(dispatch_get_main_queue(), ^{
            ax::Director::getInstance()->getScheduler()->runOnAxmolThread([ok]() { PZNotif_onPermissionResult(ok); });
        });
    }];
}

void PZNotif_nativeCancelAll() {
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center removeAllPendingNotificationRequests];
}

void PZNotif_nativeSchedule(int id, const char* title, const char* body, double delaySeconds) {
    UNMutableNotificationContent* content = [[UNMutableNotificationContent alloc] init];
    content.title = [NSString stringWithUTF8String:title];
    content.body = [NSString stringWithUTF8String:body];
    content.sound = [UNNotificationSound defaultSound];
    UNTimeIntervalNotificationTrigger* trigger =
        [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:MAX(60.0, delaySeconds) repeats:NO];
    NSString* ident = [NSString stringWithFormat:@"pz_reminder_%d", id];
    UNNotificationRequest* request = [UNNotificationRequest requestWithIdentifier:ident content:content trigger:trigger];
    [[UNUserNotificationCenter currentNotificationCenter] addNotificationRequest:request withCompletionHandler:nil];
}

#endif
