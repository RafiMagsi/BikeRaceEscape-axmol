# Permissions, capabilities and privacy checklist

Status of every feature that touches the OS, the stores or a privacy rule. "Done" = in the repo; "You" = needs an account / console / URL.

## Privacy policy URL
- **Required**: App Store Connect (App Privacy > Privacy Policy URL), Google Play Console (App content > Privacy policy) and the AdMob consent message (Privacy & messaging) all ask for a public https URL. The game cannot ship without one because it shows ads and sells IAPs.
- **In-game link: done.** Settings > PRIVACY POLICY opens `strings/PrivacyPolicyUrl` (`Content/CompleteGameInfo.plist`) = https://nextfictiongames.com/privacy-policy. iOS shows it in an in-app Safari sheet (SFSafariViewController, verified); Android opens the system browser (ACTION_VIEW, needs no permission). Android Custom Tabs would need the `androidx.browser` dependency (not added).
- **You**: paste the same URL into App Store Connect, Play Console and the AdMob consent message.

## iOS
| Feature | What it needs | State |
|---|---|---|
| AdMob banner / interstitial / rewarded | `GADApplicationIdentifier` in the built Info.plist; `SKAdNetworkItems` | Done (injected by CMake; SKAdNetwork list in Info.plist) |
| Advertising identifier (AdMob, Chartboost) | ATT prompt is only REQUIRED if the app tracks users across other companies' apps. We decided not to: **no ATT prompt and no `NSUserTrackingUsageDescription`**. Without permission the IDFA is zeroed and AdMob serves non-personalised ads (lower eCPM, no review risk). In App Store Connect answer "Data Used to Track You: none". | Done (removed) |
| Google consent (UMP) | Message created in AdMob > Privacy & messaging (GDPR + US states + IDFA explainer optional) | **You** (log still says "no form(s) configured") |
| Privacy manifest | `PrivacyInfo.xcprivacy` in the app bundle. **Mandatory**: App Store Connect rejects uploads whose code uses required-reason APIs (UserDefaults, file timestamps, boot time, disk space) without declaring them. NSPrivacyTracking = false, no tracking domains. | Done: `proj.ios_mac/ios/targets/ios/PrivacyInfo.xcprivacy` (bundled, verified). Google Mobile Ads / UMP / Chartboost ship their own. App Privacy answers: Device ID + Advertising data, used for third-party advertising, not used for tracking. |
| In-app purchases | "In-App Purchase" capability on the App ID; 8 products (remove ads, kid mode, coin1-6) created + sent for review with the build | Code done; **You** for App Store Connect |
| Game Center | Entitlement `com.apple.developer.game-center` | Done |
| Local reminders | No Info.plist key and no capability needed (UNUserNotificationCenter local); the OS prompt appears only after the in-game soft ask | Done |
| Haptics | None | Done |
| Privacy policy sheet | `SafariServices` framework | Done (linked) |
| Encryption export | `ITSAppUsesNonExemptEncryption = false` | Done |
| Orientation | Landscape only | Done |

## Android
| Feature | What it needs | State |
|---|---|---|
| Ads | `INTERNET`, `ACCESS_NETWORK_STATE`, AdMob app id meta-data | Done |
| Advertising ID (Android 13+, targetSdk 33+) | `com.google.android.gms.permission.AD_ID` + Play Console > App content > Advertising ID = Yes (Ads) | **Added now** to the manifest; **You** answer the Play Console form |
| Billing | `com.android.vending.BILLING` (also added by the Billing library); 6 coin packs (`...coin1-6`, consumable) + remove ads + kid mode created in Play Console | Code done; **You** for the products |
| Local reminders | `POST_NOTIFICATIONS` (runtime prompt on Android 13+, asked only after the soft ask). Alarms use `setAndAllowWhileIdle` (inexact), so NO `SCHEDULE_EXACT_ALARM` is needed. Reminders are re-scheduled when the app opens; they do not survive a reboot until then (no BOOT_COMPLETED receiver) | Done |
| Haptics | `VIBRATE` | Done |
| Consent (UMP) | Same AdMob message as iOS | **You** |
| Data safety form | Declare: Device or other IDs (advertising ID), App activity/Purchases (Play Billing) shared with Google AdMob; data is not sold; encrypted in transit | **You** |
| Target audience / Families | The game is NOT child-directed. Do not tick "designed for children" (that would forbid the advertising ID and most AdMob formats); the "Kids bike" is only a character/IAP | Decision for you |

## Settings screen (in-game)
Sound effects, Music, Vibration, Reminders, Privacy options (Google UMP; the row only appears where the player's region requires it, as Google's policy asks), Privacy policy (in-app page). Sound/Music are stored in UserDefaults (`PZ_SoundOn`, `PZ_MusicOn`).

## Release checklist
1. Paste the privacy policy URL into the 3 consoles (already in the game).
2. AdMob: publish the consent message (EEA/UK + US states); add the app-ads.txt to your developer website.
3. App Store Connect: App Privacy answers (above), IAP products, age rating (ads: no unrestricted web access).
4. Play Console: Data safety, Ads = yes, Advertising ID = yes, content rating, 8 products.
5. Test on a real device: ATT prompt, consent form (use `UMPDebugSettings` EEA test device), reminder permission, purchase + restore.
