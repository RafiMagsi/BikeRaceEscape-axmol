# Dino Bike Escape: Build & Device Commands

Run every command from the repo root unless a step says otherwise:

```bash
cd /Users/rafi/developer/games/remake/BikeRaceEscape-axmol
```

**Contents**

0. [Run on a device (automatic)](#0-run-on-a-device-automatic)
1. [Quick reference](#1-quick-reference)
2. [Bump the version](#2-bump-the-version)
3. [iOS build](#3-ios-build)
4. [Android build](#4-android-build)
5. [iPhone: install, run, logs, crashes](#5-iphone-install-run-logs-crashes)
6. [Troubleshooting](#6-troubleshooting)

---

## 0. Run on a device (automatic)

One script builds, installs and launches. It finds the iPhone, booted simulator or Android device by itself: with exactly one target it just uses it, with several it shows a numbered list and asks, with none it lets you pick a simulator to start. You never type a device id.

```bash
tools/run.sh                    # asks: iOS or Android, then picks / asks for the device
tools/run.sh ios                # iPhone plugged in? simulator running? picks it (asks if there are several)
tools/run.sh android            # release build (use --debug for a debug build)
tools/run.sh ios --list         # only list what it can see
tools/run.sh ios --device "iPhone 15"   # optional: choose by name part instead of being asked
tools/run.sh ios --no-build     # skip the build: install + launch the last one
tools/run.sh ios --app path/to/BikeRaceEscapeAxmol.app   # install this build, no compile
```

- **iPhone**: Release archive (section 3.2) then `devicectl` install + launch. The first run generates the Xcode project (3.1).
- **Simulator**: Debug build into `build-ios-sim`, then `simctl` install + launch (the Simulator app is opened and booted if needed).
- **Android**: `assembleRelease`, `adb install -r`, launch. Needs `adb` (on PATH or in `~/Library/Android/sdk/platform-tools`) and USB debugging on.
- Set `TEAM_ID=...` to override the Apple team (default `XF5237ER2B`).

---

## Ads: test vs live (automatic)

You do not switch anything by hand. The build type decides:

| Build | Ads |
| --- | --- |
| Xcode **Debug**, `tools/run.sh ios` to a simulator, Gradle **debug** | Google's sample ad units only (test ads, safe to tap). Log line: `AdMob testMode is ON` |
| iOS **Release / archive**, Gradle **bundleRelease / assembleRelease** | Your live unit ids from `Content/ads.plist` |

`ads.plist` keeps `testMode = false` (the release value). If it were ever `true` in a release build the log warns you. Never register your own phone as a "test device" in a release flow and tap live ads. Chartboost has no automatic test switch: use it only in release builds you do not tap.

---

## 1. Quick reference

| Item | Value |
| --- | --- |
| Bundle ID / package | `com.nextfiction.dinobikeescape` |
| Apple team | `XF5237ER2B` |
| iOS minimum version | 15.0 |
| Android minSdk | 24 (set in `.axproj`) |
| iOS engine | always `thirdparty/axmol` (the project's own copy; CMake picks it automatically, `AX_ROOT` is ignored) |
| Android engine (`AX_ROOT`) | leave as is; the default `~/development/axmol` is correct |
| Android signing key | `proj.android/dino-bike-upload.keystore` |
| Android signing settings | `proj.android/keystore.properties` (both files are git-ignored; **back them up**) |
| Test iPhone | Rafi iPhone 15 (found automatically by `tools/run.sh`; no id needed) |
| Current version | 1.4.0: iOS build 10, Android versionCode 6 |

---

## 2. Bump the version

Raise the build number every time you upload.

| Platform | File | What to edit |
| --- | --- | --- |
| iOS | `proj.ios_mac/ios/targets/ios/Info.plist` | `CFBundleShortVersionString` (version), `CFBundleVersion` (build number) |
| Android | `proj.android/app/build.gradle` | `versionName` (version), `versionCode` (must go up) |

---

## 3. iOS build

> **Note:** iOS always builds against the project's own engine copy (`thirdparty/axmol`). `cmake/modules/AXGameEngineSetup.cmake` selects it automatically on iOS, so you no longer need to set `AX_ROOT`. The shared engine (`~/development/axmol`) lacks the Apple fixes and used to break the link step with `library 'atomic' not found`.

### 3.1 Regenerate the Xcode project

Needed after changes to CMake, the Podfile, or files in `Content/`.

```bash
cmake -S . -B build-ios-device -G Xcode \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/thirdparty/axmol/1k/ios.cmake \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DPZ_DEVELOPMENT_TEAM=XF5237ER2B \
  -DDEPLOYMENT_TARGET=15.0 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 \
  -DCMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET=15.0

(cd build-ios-device && pod install)
```

### 3.2 Archive

```bash
rm -rf build-ios-device/BikeRaceEscapeAxmol.xcarchive

xcodebuild \
  -workspace build-ios-device/BikeRaceEscapeAxmol.xcworkspace \
  -scheme BikeRaceEscapeAxmol \
  -configuration Release \
  -destination 'generic/platform=iOS' \
  -archivePath build-ios-device/BikeRaceEscapeAxmol.xcarchive \
  -allowProvisioningUpdates \
  archive
```

Output: `build-ios-device/BikeRaceEscapeAxmol.xcarchive`

### 3.3 Clean build

```bash
xcodebuild \
  -workspace build-ios-device/BikeRaceEscapeAxmol.xcworkspace \
  -scheme BikeRaceEscapeAxmol \
  -configuration Release \
  -destination 'generic/platform=iOS' \
  clean

rm -rf build-ios-device/BikeRaceEscapeAxmol.xcarchive
rm -rf ~/Library/Developer/Xcode/DerivedData/BikeRaceEscapeAxmol-*
```

Then run 3.1 and 3.2 again.

### 3.4 Check the archive

```bash
APP=build-ios-device/BikeRaceEscapeAxmol.xcarchive/Products/Applications/BikeRaceEscapeAxmol.app

/usr/libexec/PlistBuddy \
  -c "Print :MinimumOSVersion" \
  -c "Print :CFBundleShortVersionString" \
  -c "Print :CFBundleVersion" \
  $APP/Info.plist

ls $APP/Frameworks
```

What you should see:

- Minimum OS `15.0` and the version you just set.
- `Frameworks` containing **only** `soft_oal.framework`. If `libswift*.dylib` files appear, the deployment target is below 15 and the app can crash on iOS 26.

### 3.5 Export and upload to App Store Connect

> Not run in this project yet; verify the first time you use it.

```bash
cat > build-ios-device/ExportOptions.plist <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>method</key><string>app-store-connect</string>
  <key>teamID</key><string>XF5237ER2B</string>
  <key>destination</key><string>upload</string>
</dict></plist>
EOF

xcodebuild -exportArchive \
  -archivePath build-ios-device/BikeRaceEscapeAxmol.xcarchive \
  -exportOptionsPlist build-ios-device/ExportOptions.plist \
  -allowProvisioningUpdates
```

---

## 4. Android build

> **Before you start**
>
> - Run the commands from the `proj.android` folder (that is where `gradlew` is).
> - Android builds with the shared engine `~/development/axmol` (ninja is installed there). If `AX_ROOT` is left pointing at the project's iOS copy (`thirdparty/axmol`), `proj.android/settings.gradle` ignores it and uses the shared engine, so you don't have to fix your terminal.
> - The SDK location comes from `proj.android/local.properties` (`sdk.dir=/Users/rafi/Library/Android/sdk`, git-ignored). You do **not** need to export `ANDROID_HOME`. If that file is missing, create it with the line above.
> - The Gradle project is named `:BikeRaceEscapeAxmol`. Do not use `:app:`.

### 4.1 Build the signed bundle (AAB)

```bash
cd /Users/rafi/developer/games/remake/BikeRaceEscape-axmol/proj.android

./gradlew :BikeRaceEscapeAxmol:bundleRelease
```

Output: `proj.android/app/build/outputs/bundle/release/BikeRaceEscapeAxmol-release.aab`

### 4.2 Clean build

```bash
cd /Users/rafi/developer/games/remake/BikeRaceEscape-axmol/proj.android

rm -rf app/.cxx app/build        # removes the native build cache and all outputs
./gradlew :BikeRaceEscapeAxmol:bundleRelease
```

### 4.3 Verify the AAB

```bash
cd proj.android
AAB=app/build/outputs/bundle/release/BikeRaceEscapeAxmol-release.aab

jarsigner -verify $AAB                          # expect: jar verified
keytool -printcert -jarfile $AAB | grep SHA256  # expect: 24:D9:29:00:46:12:15:BE:CA:F7:5E:36:41:3E:5C:49:FA:D9:7F:8D:3F:BA:07:5C:51:05:91:E4:AB:0F:FE:37
```

### 4.4 Upload

Upload by hand in Play Console: **Testing → Internal testing → Create release**, then drop in the AAB.

---

## 5. iPhone: install, run, logs, crashes

Set these once per terminal:

```bash
# the connected iPhone, found automatically (no id to remember)
xcrun devicectl list devices --json-output /tmp/devices.json >/dev/null
DEV=$(python3 -c "import json;print(next(d['identifier'] for d in json.load(open('/tmp/devices.json'))['result']['devices'] if d['connectionProperties'].get('tunnelState')=='connected'))")
BID=com.nextfiction.dinobikeescape
APP=build-ios-device/BikeRaceEscapeAxmol.xcarchive/Products/Applications/BikeRaceEscapeAxmol.app
```

### Install and launch

```bash
xcrun devicectl device install app --device $DEV $APP
xcrun devicectl device process launch --device $DEV --terminate-existing $BID
```

### Stop the app

```bash
xcrun devicectl device info processes --device $DEV | grep BikeRace      # find the pid
xcrun devicectl device process terminate --device $DEV --pid <pid>
```

### Game log from the last launch

The log file is truncated on every launch, so read it before relaunching.

```bash
xcrun devicectl device copy from --device $DEV \
  --domain-type appDataContainer --domain-identifier $BID \
  --source Documents/pz_run_log.txt --destination /tmp/run.txt

grep -n "Remove ads\|COPPA\|banner\|interstitial\|AdsController" /tmp/run.txt
```

### Crash reports

```bash
xcrun devicectl device info files --device $DEV --domain-type systemCrashLogs | grep BikeRace

xcrun devicectl device copy from --device $DEV --domain-type systemCrashLogs \
  --source <name>.ips --destination /tmp/crash.ips
```

### Symbolicate a crash

Use the dSYM from the archive that matches the build that crashed. The address is `0x100000000` plus the frame's `imageOffset`, written in hex.

```bash
DSYM=build-ios-device/BikeRaceEscapeAxmol.xcarchive/dSYMs/BikeRaceEscapeAxmol.app.dSYM/Contents/Resources/DWARF/BikeRaceEscapeAxmol

atos -o $DSYM -arch arm64 -l 0x100000000 $(printf '0x%x' $((0x100000000 + <imageOffset>)))
```

### Saved settings on the phone (ads and purchases)

`Documents/UserDefault.bin` holds `PMP_RemoveAds`, `PMP_KidMode`, `PMP_KidModePurchased` and the coins. **If ads stop showing, check these first**: `PMP_RemoveAds = true` switches ads off by design.

```bash
xcrun devicectl device copy from --device $DEV \
  --domain-type appDataContainer --domain-identifier $BID \
  --source Documents/UserDefault.bin --destination /tmp/UserDefault.bin
```

Back the file up before changing it, and stop the app before copying a changed file back (`copy to`).

---

## 6. Troubleshooting

| Symptom | Cause | Fix |
| --- | --- | --- |
| `ld: library 'atomic' not found` | The project was generated against `~/development/axmol` (older CMake files, before the automatic engine selection) | Regenerate with 3.1; check `grep -c -- -latomic build-ios-device/BikeRaceEscapeAxmol.xcodeproj/project.pbxproj` prints `0`, then archive again |
| App aborts at launch on iPhone with "libswiftCore requires an OS version prior to 12.2" | Deployment target below 15, so Xcode embedded old Swift libraries | Use the 15.0 flags in 3.1 and check `Frameworks` in 3.4 |
| `animation_sN.png.tmp.png couldn't be opened` | Another process was rewriting `Content/Images` when CMake ran | Re-run 3.1, then archive again |
| `Duplicate resources … ic_launcher` (Android) | Icons generated into `app/src/main/res` | Generator now writes to `app/res`; delete `app/src/main/res` if it reappears |
| `No suitable ninja found` (Android) | ninja missing from `~/development/axmol/tools/external/ninja` | Install ninja 1.10 to 1.13.2 into that folder |
| `Couldn't find android sdk. Specify path in local.properties or system env ANDROID_HOME` | No SDK location set | Create `proj.android/local.properties` containing `sdk.dir=/Users/rafi/Library/Android/sdk` |
| `Project ':app' not found` or `Cannot locate tasks that match ':app:bundleRelease'` | Wrong project name | Use `:BikeRaceEscapeAxmol:bundleRelease` |
| `./gradlew: No such file or directory` | Not in the `proj.android` folder | `cd` to `proj.android` first |
| `Cannot run program …/thirdparty/axmol/tools/external/ninja/ninja` / `No suitable ninja found` | `AX_ROOT` pointed at the iOS engine copy, which has no ninja | Fixed in `settings.gradle` (it now switches to `~/development/axmol`). If you still see it, run `git status proj.android/settings.gradle` to make sure the change is there |
| Gradle fails right away with another engine or `AX_ROOT` error | `AX_ROOT` is unset in that shell | `export AX_ROOT=$HOME/development/axmol`, or open a fresh terminal |
| Play Console says minimum SDK is too low | `min_sdk` in `.axproj` | Keep `min_sdk=24` |
| No ads on the phone | `PMP_RemoveAds` is `true` in the saved settings | Check `UserDefault.bin` (section 5) |
| Command never returns | macOS has no `timeout` command | Run long commands in the background |
