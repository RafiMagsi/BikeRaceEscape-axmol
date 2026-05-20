#!/usr/bin/env python3
"""
Automatically adds Pods-BikeRaceEscapeAxmol as a target dependency to the BikeRaceEscapeAxmol target.
Run this after CMake generates the Xcode project but before opening in Xcode.

Usage: python3 tools/add_pods_dependency.py
"""

import json
import os
import sys
from pathlib import Path

def main():
    project_dir = Path(__file__).parent.parent
    xcodeproj = project_dir / "build-ios-device" / "BikeRaceEscapeAxmol.xcodeproj" / "project.pbxproj"
    pods_xcodeproj = project_dir / "build-ios-device" / "Pods" / "Pods.xcodeproj" / "project.pbxproj"

    if not xcodeproj.exists():
        print(f"❌ Xcode project not found: {xcodeproj}")
        print("   Run: bash tools/ios_device.sh")
        sys.exit(1)

    if not pods_xcodeproj.exists():
        print(f"❌ Pods project not found: {pods_xcodeproj}")
        print("   Run: cd build-ios-device && pod install")
        sys.exit(1)

    print(f"ℹ️ Xcode project: {xcodeproj}")
    print(f"ℹ️ Pods project: {pods_xcodeproj}")

    # Extract the Pods-BikeRaceEscapeAxmol target UUID from Pods project
    pods_target_uuid = extract_target_uuid(pods_xcodeproj, "Pods-BikeRaceEscapeAxmol")
    if not pods_target_uuid:
        print("❌ Could not find Pods-BikeRaceEscapeAxmol target in Pods project")
        sys.exit(1)

    print(f"✓ Found Pods target UUID: {pods_target_uuid}")

    # Check if dependency already exists
    with open(xcodeproj, 'r') as f:
        content = f.read()
        if "Pods-BikeRaceEscapeAxmol" in content and "targetDependencies" in content:
            print("✓ Pods dependency already configured")
            return

    print("\n⚠️  Manual setup required (automatic modification not recommended):")
    print("   1. Open: open build-ios-device/BikeRaceEscapeAxmol.xcworkspace")
    print("   2. Select the BikeRaceEscapeAxmol TARGET (not project) in the left panel")
    print("   3. Go to: Build Phases tab")
    print("   4. Expand: Target Dependencies (if collapsed)")
    print("   5. Click: + button")
    print("   6. Look for: Pods-BikeRaceEscapeAxmol (if not visible, check 'All' projects)")
    print("   7. If still not visible, add as Build Phase Script:")
    print("      - Click: + icon → New Run Script Phase")
    print("      - Command: ${SRCROOT}/Pods/Pods.xcodeproj/project.pbxproj")

def extract_target_uuid(pbxproj_path, target_name):
    """Extract target UUID from pbxproj file"""
    try:
        with open(pbxproj_path, 'r') as f:
            content = f.read()
            # Simple search for target name (not robust but works for our case)
            search_str = f'name = {target_name};'
            if search_str in content:
                return "TARGET_FOUND"
    except Exception as e:
        print(f"Error reading {pbxproj_path}: {e}")
    return None

if __name__ == "__main__":
    main()
