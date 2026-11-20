//
//  V2Leaderboard.h
//  Daily leaderboard facade (PRODUCT_PLAN 20.10 #1): Game Center on iOS, Google Play Games Services on Android. Board ids come from
//  CompleteGameInfo.plist (v2.leaderboards.iosDaily / androidDaily); empty id = feature off (every call is a no-op).
//  Create the boards in App Store Connect (a RECURRING daily leaderboard, best score) and Play Console, then paste the ids.
//  Scores are submitted by the client: the platform does not verify them (see PRODUCT_PLAN 23.16).
//
#pragma once

namespace V2 {
namespace Leaderboard {

bool configured();                 // an id exists for this platform
void submitDaily(int score);       // ranked daily result (the platform keeps the best of the day)
void showDaily();                  // open the platform leaderboard UI

} // namespace Leaderboard
} // namespace V2
