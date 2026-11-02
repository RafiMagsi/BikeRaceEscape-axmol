# Android bundle optimization

## Done
- [x] Full Mode: R8 full mode pinned (`android.enableR8.fullMode=true`) and `proguard-android-optimize.txt` enabled.
- [x] Resource Shrinking: `shrinkResources = true`, `minifyEnabled = true`, `resConfigs` limited to `en`.
- [x] Removed unused content from the build:
  - `Content/Images/SD` moved to `development/unused-content/SD` (HD sprite sheets are pixel-identical; HD direct assets are higher resolution).
  - Removed duplicate `GeneralApplicationSplashScreen_0_0.png`.
  - Removed `.DS_Store` / `.dummy` files and excluded them (plus `*.md`) from the Gradle asset copy.
- [x] Play Billing upgraded from 7.1.1 to 8.0.0.
- [x] Result: AAB 69 MB -> ~43 MB; APK ~70 MB -> ~43 MB.

## Remaining
- [ ] Optimized resource shrinking: requires AGP 9.3+ (current AGP 8.11.1).
- [ ] Repackage classes: blocked by reflection in AdMob/Billing SDKs; needs targeted keep rules first.
- [ ] Upgrade to AGP 9.0: separate migration task.
- [ ] Optional: re-encode the HD sprite sheets (19.6 MB) as high-quality WebP to cut another ~15 MB.
