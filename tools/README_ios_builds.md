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

Example:
```bash
CONFIG=Release TARGET=BikeRaceEscapeAxmol tools/ios_device.sh
```
