# iOS builds (Axmol / CMake)

This repo uses separate build directories for iOS **device** and **simulator**.
They are not interchangeable (a device build cannot link/run on the simulator and vice versa).

## One-liners

### Simulator (Apple Silicon, default)
```bash
tools/ios_sim.sh
```

### Simulator (reset build dir)
```bash
FORCE_CLEAN=1 tools/ios_sim.sh
```

### Simulator (Intel)
```bash
SIM_ARCH=x86_64 tools/ios_sim.sh
```

### Device
```bash
tools/ios_device.sh
```

If CocoaPods are installed (AdMob), build from the workspace:
```bash
open build-ios-device/BikeRaceEscapeAxmol.xcworkspace
```

### Device (configure only, no build)
```bash
CONFIGURE_ONLY=1 tools/ios_device.sh
```

Note: to run on a real device, Xcode must sign the app:
- Open the workspace
- Select the `BikeRaceEscapeAxmol` target → Signing & Capabilities
- Pick your Team and let Xcode manage the provisioning profile

## If you see “Library 'Pods-BikeRaceEscapeAxmol' not found”

This happens when the app target’s build products directory differs from the Pods aggregate library output.
With CocoaPods installed, we use Xcode’s default `$(BUILD_DIR)/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)`
so the linker can find `libPods-BikeRaceEscapeAxmol.a`.

Fix (from scratch):
```bash
FORCE_CLEAN=1 CONFIGURE_ONLY=1 tools/ios_device.sh
cd build-ios-device && pod install
open build-ios-device/BikeRaceEscapeAxmol.xcworkspace
```
Then in Xcode: `Product -> Clean Build Folder`, build again.

## If you see Swift duplicate-class warnings / “requires an OS version prior to 12.2”

That means Swift stdlib dylibs were embedded into the app bundle. On modern iOS,
Swift stdlibs are provided by the OS, and bundling them can break runtime loading.

Fix:
1) Regenerate the project:
```bash
FORCE_CLEAN=1 tools/ios_device.sh
```
2) Reinstall pods:
```bash
cd build-ios-device && pod install
```
3) Clean build in Xcode (Shift+Cmd+K) and run from the workspace.

### Device (reset build dir)
```bash
FORCE_CLEAN=1 tools/ios_device.sh
```

## Environment overrides

Both scripts support:
- `CONFIG` (default: `Debug`)
- `TARGET` (default: `BikeRaceEscapeAxmol`)
- `BUILD_DIR` (default: `build-ios-sim` / `build-ios-device`)
- `TOOLCHAIN_FILE` (default: `/Users/rafi/development/axmol/1k/ios.cmake`)
- `CONFIGURE_ONLY=1` (skip the build step)

Example:
```bash
CONFIG=Release TARGET=BikeRaceEscapeAxmol tools/ios_device.sh
```
