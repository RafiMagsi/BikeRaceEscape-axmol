# Bike Race Escape → Axmol 2.11.3 migration package

This bundle is a **core-port package** for the original Cocos2d-x 2.2.3 project.
It is not presented as a finished App Store-ready migration. The old codebase contains commercial/native plugin layers that must be modernized separately.

## What was migrated

- `Classes/` moved to `Source/`
- `Resources/` moved to `Content/`
- Old Cocos2d-x 2.x type names migrated toward Axmol/Cocos4 style
- Singleton accessors updated (`sharedDirector()` → `getInstance()`, etc.)
- Legacy callbacks partially modernized (`menu_selector` → `AX_CALLBACK_1`)
- `AppDelegate` rewritten for Axmol startup
- A desktop `main.cpp` was added
- Legacy Box2D source preserved in `thirdparty/legacy-box2d/`
- A compatibility header was added: `Source/PZLegacyCompat.h`
- A legacy input bridge was added: `PZLegacyInputLayer.*`
- Obsolete GL debug drawing was stubbed so it does not block the first engine port

## Deliberately not wired into the first build

- Old iOS native project structure
- Soomla / StoreBridge commerce integration
- Neocortex ad integration
- Old iOS Game Center / AdsManager glue

These are in `legacy_native_ios_reference/` for later reimplementation.

## Mac build command

Install/clone Axmol 2.11.3, then:

```bash
export AX_ROOT="$HOME/Development/axmol"
cd BikeRaceEscape-Axmol-2.11.3
cmake -S . -B build-macos -G Xcode -DAX_ROOT="$AX_ROOT"
cmake --build build-macos --config Debug
```

## Expected next pass

Because the source started at Cocos2d-x **2.2.3**, the first build may still expose API-level compile errors in:

- legacy label/menu factory signatures
- dictionary/array APIs
- file utility APIs
- platform-specific macro branches
- old Box2D/debug draw edge cases

That compiler log is the correct input for the final pass. Do not try to manually fix random errors one-by-one without a systematic build log.
