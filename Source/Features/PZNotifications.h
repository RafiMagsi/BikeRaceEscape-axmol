//
//  PZNotifications.h
//  Local reminder notifications (free gift ready, daily reward, missions, streak guard, comebacks).
//
//  Design: the OS permission prompt is a one-shot resource, so it is never shown at launch. The game first shows its
//  own "remind me?" offer once the player has shown interest (finished a few runs, then a happy moment such as a new
//  best or a claimed reward). Only a "yes" there triggers the OS dialog. A "not now" is remembered and the offer comes
//  back later with different copy, up to a limit. Messages, gaps and pacing come from the `notifications` section of
//  CompleteGameInfo.plist (see PZGameConfig.h for the defaults).
//
#pragma once

#include "PZLegacyCompat.h"
#include <functional>

namespace PZNotif {

enum class Status { Unknown = 0, NotDetermined = 1, Granted = 2, Denied = 3 };

Status status();                 // cached; refreshed in the background
void refreshStatus();
void requestPermission(std::function<void(bool granted)> done);   // shows the OS dialog (first time only)

// Cancels everything and re-schedules the reminder slots from "now" (call when the app goes to the background and
// after anything that changes what is waiting for the player).
void reschedule();
void cancelAll();

// Is it a good moment to offer reminders? (enough engagement, not too soon after the last "not now").
bool shouldOffer();
// Shows the offer popup on `parent` when shouldOffer(); returns true when it was shown.
bool maybeOffer(ax::Node* parent);
// Settings toggle: turning on re-runs the offer / permission flow as needed.
void setEnabledByPlayer(ax::Node* parent, bool on);

} // namespace PZNotif

// ---- implemented per platform -----------------------------------------------------------------
int  PZNotif_nativeStatus();                                 // PZNotif::Status as int (cached)
void PZNotif_nativeRefresh();
void PZNotif_nativeRequest();                                // result arrives through PZNotif_onPermissionResult
void PZNotif_nativeCancelAll();
void PZNotif_nativeSchedule(int id, const char* title, const char* body, double delaySeconds);
void PZNotif_onPermissionResult(bool granted);               // call on the game thread
