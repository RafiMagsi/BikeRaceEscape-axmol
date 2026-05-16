# Patch 02 — Box2D include layout fix

## Fixed from `axmol-build-02.log`
The build stopped at:

```
thirdparty/legacy-box2d/Dynamics/b2WorldCallbacks.cpp:19:10: fatal error: 'Box2D/Dynamics/b2WorldCallbacks.h' file not found
```

## Why it happened
The old Box2D source files include headers using paths such as:

```
#include <Box2D/Dynamics/b2WorldCallbacks.h>
```

But the previous migration package placed the old Box2D tree at:

```
thirdparty/legacy-box2d/
```

That layout cannot satisfy the `Box2D/...` include prefix.

## What changed
- Added a compile-facing Box2D tree at:

```
thirdparty/Box2D/
```

- Updated `CMakeLists.txt` to compile:

```
thirdparty/Box2D/*.cpp
```

- Updated include roots to:

```
thirdparty
```

This makes includes such as `Box2D/Dynamics/...` resolve correctly.

## Rebuild cleanly
Delete the previous Xcode/CMake build folder before rebuilding:

```bash
cd /Users/rafi/developer/games/remake/BikeRaceEscape-Axmol-2.11.3
rm -rf build-macos
cmake -S . -B build-macos -G Xcode -DAX_ROOT=/Users/rafi/development/axmol
cmake --build build-macos --config Debug 2>&1 | tee axmol-build-03.log
```
