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

## 6) Sanity checks (no more “Pods not found” / Swift dylib issues)

After a successful build:

- The app should **not** embed Swift stdlibs in `BikeRaceEscapeAxmol.app/Frameworks`
  (modern iOS provides them). If you see `libswift*.dylib` inside the app, you’re
  building an old generated project; regenerate from step 1.

## 7) AdMob test checklist

Use **test ad unit ids first** to validate integration, then swap to real ones:

- Make sure `Content/ads.plist` enables AdMob and has:
  - `Constants:AdMob:appIdIOS`
  - Banner/Interstitial/Rewarded unit ids for the contexts you use.
- Launch the app, reach a screen where AdsController should show a banner.
- Watch the Xcode console for AdMob logs (load/show).

