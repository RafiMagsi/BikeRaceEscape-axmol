# Patch 09 — iOS focus + legacy container compatibility

## Target shift
Build feedback should now target iOS Simulator / iOS device rather than macOS.

Recommended first compile loop:
```bash
axmol -p ios -a 'arm64' -sdk 'simulator' -c -f
```
Then open the generated Xcode project and build, or capture an xcodebuild log from the generated project.

## Error families fixed from axmol-build-08.log
- Missing `ax::__String`
- Missing `ax::__Array`
- Missing `ax::__Dictionary`
- Follow-on failures in shared data model headers caused by these removed Cocos 2.x containers

## Proactive compatibility work
- Added legacy `__String`, `__Array`, and `__Dictionary` adapters in `Source/PZLegacyCompat.h`
- Added plist loading bridge using `FileUtils::getValueMapFromFile`
- Added `CCARRAY_FOREACH`
- Added `CC_SYNTHESIZE`
- Added `SizeMake` / `RectMake`
- Added `namespace cocos2d = ax` for surviving old Parallax headers
- Added `__Array::removeObjectsInArray`
- Replaced `CCUserDefault::sharedUserDefault()` with `ax::UserDefault::getInstance()`
