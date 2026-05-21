# iOS Device Runbook (Axmol + CocoaPods AdMob)

This is the fastest “from zero to running on device” path for this repo.

## 0) Prereqs

- Xcode installed and selected: `xcode-select -p` should point to Xcode.
- CocoaPods installed: `pod --version`
- You have an Apple Development Team ID (e.g. `XF5237ER2B`).
- `Content/ads.plist` contains your real AdMob `appIdIOS` and ad unit ids.

## 1) Generate a clean device build folder

From repo root:

```bash
FORCE_CLEAN=1 CONFIGURE_ONLY=1 TEAM_ID=XF5237ER2B tools/ios_device.sh
```

Notes:
- `CONFIGURE_ONLY=1` avoids CLI builds/signing problems and lets Xcode own signing.
- `TEAM_ID=...` bakes the team into the generated project so Xcode doesn’t keep asking.

## 2) Install pods into `build-ios-device`

```bash
cd build-ios-device
pod install
```

## 3) Open the workspace (NOT the project)

```bash
open BikeRaceEscapeAxmol.xcworkspace
```

## 4) Xcode signing (one-time)

In Xcode workspace:

- Target: `BikeRaceEscapeAxmol`
- Signing & Capabilities:
  - “Automatically manage signing”: ON
  - Team: “NextFiction” (or your team)

## 5) Clean + run

- `Product -> Clean Build Folder` (Shift+Cmd+K)
- Select your iPhone device
- Run (Cmd+R)

Important: after `pod install`, do NOT re-run CMake configure into `build-ios-device/` unless you also re-run `pod install`.
Reconfiguring regenerates `BikeRaceEscapeAxmol.xcodeproj` and can wipe CocoaPods’ integration.
The script `tools/ios_device.sh` detects an existing Pods workspace and will skip reconfigure by default.

## 6) Sanity checks (no more “Pods not found” / Swift dylib issues)

After a successful build:

- If AdMob (Swift-based pods) is linked, the executable will reference `@rpath/libswiftCore.dylib`.
  In that case the **built app bundle must contain Swift stdlibs** under `BikeRaceEscapeAxmol.app/Frameworks`,
  otherwise the app can crash on device at launch with:
  `dyld: Library not loaded: @rpath/libswiftCore.dylib`

Verify the build product (DerivedData, not `build-ios-device/bin/...`):
```bash
APP_DIR="$(ls -dt ~/Library/Developer/Xcode/DerivedData/BikeRaceEscapeAxmol-* | head -1)/Build/Products/Debug-iphoneos/BikeRaceEscapeAxmol.app"
ls -1 "$APP_DIR/Frameworks" | head
```

You must see `libswiftCore.dylib` (not only `libswiftCore.dylib.original`).
If you only see `*.original`, do **Product -> Clean Build Folder**, rebuild from the `.xcworkspace`,
delete the app from the device, then reinstall.

Important: do NOT run `xcodebuild` and an Xcode GUI build at the same time; it can lock
DerivedData's `build.db` and leave a half-built `.app` (missing Frameworks content).

If you build via CLI, prefer the script which uses an isolated DerivedData folder:
```bash
TEAM_ID=XF5237ER2B tools/ios_device.sh
```

## 7) AdMob test checklist

Use **test ad unit ids first** to validate integration, then swap to real ones:

- Make sure `Content/ads.plist` enables AdMob and has:
  - `Constants:AdMob:appIdIOS`
  - Banner/Interstitial/Rewarded unit ids for the contexts you use.
- Launch the app, reach a screen where AdsController should show a banner.
- Watch the Xcode console for AdMob logs (load/show).
