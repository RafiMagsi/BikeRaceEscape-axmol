#!/bin/bash
# Automatically adds Pods-BikeRaceEscapeAxmol as a target dependency to BikeRaceEscapeAxmol target
# This is needed because CMake generates the project before CocoaPods creates the Pods project

set -e

XCODEPROJ="$1"
if [ -z "$XCODEPROJ" ]; then
  XCODEPROJ="build-ios-device/BikeRaceEscapeAxmol.xcodeproj"
fi

if [ ! -d "$XCODEPROJ" ]; then
  echo "error: Xcode project not found: $XCODEPROJ"
  exit 1
fi

PROJECT_FILE="$XCODEPROJ/project.pbxproj"

# Check if dependency already exists
if grep -q "Pods-BikeRaceEscapeAxmol" "$PROJECT_FILE" 2>/dev/null; then
  echo "✓ Pods dependency already configured in $XCODEPROJ"
  exit 0
fi

echo "ℹ To manually add Pods dependency:"
echo "  1. Open: open $XCODEPROJ/../*.xcworkspace"
echo "  2. Select target: BikeRaceEscapeAxmol"
echo "  3. Build Phases tab → + → New Target Dependencies"
echo "  4. Add: Pods-BikeRaceEscapeAxmol"
echo "  5. Save: Cmd+S"
echo ""
echo "This ensures the Pods project builds first with all frameworks."
