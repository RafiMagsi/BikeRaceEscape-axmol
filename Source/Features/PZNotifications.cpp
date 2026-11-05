//
//  PZNotifications.cpp
//
#include "PZNotifications.h"
#include "PZGameConfig.h"
#include "PZProgress.h"
#include "PZPopups.h"
#include "PZUI.h"
#include "PZLayout.h"

#include <ctime>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif

USING_NS_AX;

namespace PZNotif {

namespace {
std::function<void(bool)> g_permissionCb;

// Seconds from now until the next occurrence of `hour`:00 local time that is at least `minSeconds` away.
double secondsUntilLocalHour(int hour, double minSeconds) {
    std::time_t now = std::time(nullptr);
    std::time_t earliest = now + static_cast<std::time_t>(minSeconds);
    std::tm lt{};
#if defined(_WIN32)
    localtime_s(&lt, &earliest);
#else
    localtime_r(&earliest, &lt);
#endif
    lt.tm_hour = hour;
    lt.tm_min = 0;
    lt.tm_sec = 0;
    std::time_t target = std::mktime(&lt);
    if (target < earliest) target += 24 * 3600;
    return static_cast<double>(target - now);
}
} // namespace

Status status() {
    return static_cast<Status>(PZNotif_nativeStatus());
}

void refreshStatus() { PZNotif_nativeRefresh(); }

void requestPermission(std::function<void(bool)> done) {
    g_permissionCb = std::move(done);
    PZNotif_nativeRequest();
}

void cancelAll() { PZNotif_nativeCancelAll(); }

void reschedule() {
    PZCfg::ensureLoaded();
    PZNotif_nativeCancelAll();
    auto* pg = PZProgress::shared();
    if (!PZCfg::kEnableNotifications || !pg->notifEnabled()) return;
    if (status() != Status::Granted) return;

    const int day = PZProgress::today();
    int id = 1;
    int slotIndex = 0;
    for (const auto& slot : PZCfg::kNotifSlots) {
        ++slotIndex;
        if (pg->streak() < slot.minStreak) continue;
        // A reminder for something the player cannot yet earn again would only be noise: the daily reminder is
        // skipped when the reward is still unclaimed today (they are in the game right now).
        double delay = static_cast<double>(slot.hours) * 3600.0;
        if (slot.localHour >= 0) delay = secondsUntilLocalHour(slot.localHour, delay);
        const auto& msg = slot.messages[static_cast<size_t>(day + slotIndex) % slot.messages.size()];
        PZNotif_nativeSchedule(id++, msg.title.c_str(), msg.body.c_str(), delay);
    }
}

bool shouldOffer() {
    if (!PZCfg::kEnableNotifications) return false;
    auto* pg = PZProgress::shared();
    if (!pg->notifEnabled()) return false;
    const Status st = status();
    if (st == Status::Granted || st == Status::Denied) return false;    // nothing to ask / cannot ask again
    if (pg->totalRuns() < PZCfg::kNotifMinRuns) return false;
    if (pg->notifAsks() >= PZCfg::kNotifMaxAsks) return false;
    if (pg->notifAsks() > 0 && pg->daysSinceNotifAsk() < PZCfg::kNotifAskCooldownDays) return false;
    return true;
}

bool maybeOffer(Node* parent) {
    if (!parent || !shouldOffer()) return false;
    PZPopups::showNotificationOffer(parent);
    return true;
}

void setEnabledByPlayer(Node* parent, bool on) {
    auto* pg = PZProgress::shared();
    pg->setNotifEnabled(on);
    if (!on) {
        cancelAll();
        return;
    }
    const Status st = status();
    if (st == Status::Granted) {
        reschedule();
    } else if (st == Status::Denied) {
        PZUI::banner(parent, PZCfg::kStrNotifBtn, PZCfg::kStrNotifDenied, PZCfg::kColAccent2, Director::getInstance()->getWinSize().height * 0.55f, 1.6f, 1200);
    } else {
        PZPopups::showNotificationOffer(parent);
    }
}

} // namespace PZNotif

// Platform glue ------------------------------------------------------------------------------------------------------

void PZNotif_onPermissionResult(bool granted) {
    PZNotif::refreshStatus();
    auto cb = std::move(PZNotif::g_permissionCb);
    PZNotif::g_permissionCb = nullptr;
    if (granted) PZNotif::reschedule();
    if (cb) cb(granted);
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

namespace {
constexpr const char* kBridge = "dev/axmol/app/NotificationBridge";
}

int PZNotif_nativeStatus() {
    return ax::JniHelper::callStaticBooleanMethod(kBridge, "hasPermission") ? static_cast<int>(PZNotif::Status::Granted)
                                                                          : static_cast<int>(ax::JniHelper::callStaticBooleanMethod(kBridge, "canAskAgain")
                                                                                                 ? PZNotif::Status::NotDetermined
                                                                                                 : PZNotif::Status::Denied);
}
void PZNotif_nativeRefresh() {}
void PZNotif_nativeRequest() {
    ax::JniHelper::callStaticVoidMethod(kBridge, "requestPermission");
    // The system dialog reports back asynchronously and the activity does not forward the result here: poll briefly.
    auto* sched = Director::getInstance()->getScheduler();
    auto elapsed = std::make_shared<float>(0.f);
    static int key = 0;
    sched->schedule([elapsed, sched](float dt) {
        *elapsed += dt;
        const bool granted = ax::JniHelper::callStaticBooleanMethod(kBridge, "hasPermission");
        if (granted || *elapsed > 45.f) {
            sched->unschedule("pz_notif_poll", &key);
            PZNotif_onPermissionResult(granted);
        }
    }, &key, 0.5f, false, "pz_notif_poll");
}
void PZNotif_nativeCancelAll() { ax::JniHelper::callStaticVoidMethod(kBridge, "cancelAll"); }
void PZNotif_nativeSchedule(int id, const char* title, const char* body, double delaySeconds) {
    ax::JniHelper::callStaticVoidMethod(kBridge, "schedule", id, std::string(title), std::string(body),
                                        static_cast<jlong>(delaySeconds * 1000.0));
}

#elif !(CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

int  PZNotif_nativeStatus() { return static_cast<int>(PZNotif::Status::Denied); }
void PZNotif_nativeRefresh() {}
void PZNotif_nativeRequest() { PZNotif_onPermissionResult(false); }
void PZNotif_nativeCancelAll() {}
void PZNotif_nativeSchedule(int, const char*, const char*, double) {}

#endif
