# App icons

This repo intentionally keeps **generated** icon PNGs out of version control until you decide on a final icon.

## Generate

1. Put the source icon PNG somewhere (recommended: `art/icon-source.png`), ideally **1024x1024**.
2. Run:

```bash
python3 tools/generate_app_icons.py --input art/icon-source.png
```

## iOS

Generated here:
- `platform/ios/Assets.xcassets/AppIcon.appiconset`

## Android

Generated here (Gradle-style path for later Android packaging):
- `platform/android/app/src/main/res/mipmap-*`

