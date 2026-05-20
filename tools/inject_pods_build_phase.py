#!/usr/bin/env python3
"""
Injects a Run Script build phase to build Pods first.
This modifies the Xcode project.pbxproj file safely.
"""

import os
import re
import sys
import uuid
from pathlib import Path

def generate_pbx_id():
    """Generate a valid Xcode PBX ID (24 hex chars)"""
    return ''.join(f'{uuid.uuid4().int:032x}'[:24])

def inject_pods_build_phase(project_path):
    """Inject a Run Script build phase to build Pods first"""

    pbxproj_path = project_path / "project.pbxproj"

    if not pbxproj_path.exists():
        print(f"❌ project.pbxproj not found at {pbxproj_path}")
        return False

    with open(pbxproj_path, 'r') as f:
        content = f.read()

    # Check if already injected
    if "Build Pods First" in content:
        print("✓ Pods build phase already injected")
        return True

    # Generate new IDs
    shell_phase_id = generate_pbx_id()

    # Shell script command (properly escaped for pbxproj)
    shell_script = (
        "set -e\\n"
        "echo \\\"Building Pods...\\\"\\n"
        "cd \\\"${SRCROOT}\\\"\\n"
        "xcodebuild -workspace BikeRaceEscapeAxmol.xcworkspace "
        "-scheme Pods-BikeRaceEscapeAxmol "
        "-configuration ${CONFIGURATION} "
        "-arch ${ARCHS} "
        "-sdk ${SDK} "
        "SYMROOT=${SYMROOT} "
        "OBJROOT=${OBJROOT} "
        "> /dev/null 2>&1\\n"
        "echo \\\"✓ Pods built successfully\\\""
    )

    # Create the shell script phase definition
    shell_phase_def = f"""{shell_phase_id} /* Build Pods First */ = {{
\t\tisa = PBXShellScriptBuildPhase;
\t\tbuildActionMask = 2147483647;
\t\tfiles = ();
\t\tinputFileListPaths = ();
\t\tinputPaths = ();
\t\tname = "Build Pods First";
\t\toutputFileListPaths = ();
\t\toutputPaths = ();
\t\trunOnlyForDeploymentPostprocessing = 0;
\t\tshellPath = /bin/sh;
\t\tshellScript = "{shell_script}";
\t\tshowEnvVarsInLog = 0;
\t}};"""

    # Find where to insert the shell phase definition
    # Look for existing PBXShellScriptBuildPhase section
    shell_section_match = re.search(r'/\* Begin PBXShellScriptBuildPhase section \*/(.*?)/\* End PBXShellScriptBuildPhase section \*/', content, re.DOTALL)

    if shell_section_match:
        # Insert into existing section
        section_content = shell_section_match.group(1)
        new_section = section_content + "\t" + shell_phase_def + "\n\t"
        content = content.replace(shell_section_match.group(0), f"/* Begin PBXShellScriptBuildPhase section */{new_section}/* End PBXShellScriptBuildPhase section */")
    else:
        # Create new section
        new_section = f"/* Begin PBXShellScriptBuildPhase section */\n\t{shell_phase_def}\n/* End PBXShellScriptBuildPhase section */\n\n"
        # Insert before the last section
        last_end_pos = content.rfind("/* End")
        if last_end_pos > 0:
            content = content[:last_end_pos] + new_section + content[last_end_pos:]
        else:
            print("❌ Could not find insertion point")
            return False

    # Find the BikeRaceEscapeAxmol target and add the phase to its buildPhases
    # Look for: buildPhases = ( ... ); in the BikeRaceEscapeAxmol target

    # Find all buildPhases arrays
    pattern = r'(buildPhases = \(([^)]+)\);)'
    matches = list(re.finditer(pattern, content, re.DOTALL))

    if not matches:
        print("❌ Could not find buildPhases in project")
        return False

    # We need to find the right target's buildPhases (the BikeRaceEscapeAxmol app target)
    # Look backwards from the match to find which target owns this buildPhases
    modified = False
    for match in matches:
        # Look backwards to see if this belongs to BikeRaceEscapeAxmol target
        before_match = content[:match.start()]
        # Find the last occurrence of 'isa = PBXNativeTarget;' before this match
        last_target_start = before_match.rfind('isa = PBXNativeTarget;')
        if last_target_start > 0:
            # Check if this target is BikeRaceEscapeAxmol (look for name = BikeRaceEscapeAxmol)
            target_section = content[last_target_start:match.end() + 100]
            if 'name = BikeRaceEscapeAxmol;' in target_section or 'name = "BikeRaceEscapeAxmol";' in target_section:
                # This is our target! Insert the phase
                build_phases_str = match.group(2)
                new_build_phases = f"{shell_phase_id} /* Build Pods First */, {build_phases_str}"
                old_match = match.group(1)
                new_match = old_match.replace(build_phases_str, new_build_phases)
                content = content.replace(old_match, new_match, 1)
                modified = True
                break

    if not modified:
        print("❌ Could not find BikeRaceEscapeAxmol target buildPhases")
        return False

    # Write back
    with open(pbxproj_path, 'w') as f:
        f.write(content)

    print(f"✓ Injected Pods build phase (ID: {shell_phase_id})")
    return True

def main():
    project_dir = Path(__file__).parent.parent
    xcodeproj = project_dir / "build-ios-device" / "BikeRaceEscapeAxmol.xcodeproj"

    if not xcodeproj.exists():
        print(f"❌ Xcode project not found: {xcodeproj}")
        print("   Run: bash tools/ios_device.sh")
        sys.exit(1)

    if inject_pods_build_phase(xcodeproj):
        print("\n✅ Build phase injected successfully!")
        print("\nClose Xcode if open, then rebuild:")
        print("  1. open build-ios-device/BikeRaceEscapeAxmol.xcworkspace")
        print("  2. Cmd+Shift+K (Clean Build Folder)")
        print("  3. Cmd+B (Build)")
        print("  4. Cmd+R (Run on device)")
        sys.exit(0)
    else:
        print("\n❌ Failed to inject build phase")
        sys.exit(1)

if __name__ == "__main__":
    main()
