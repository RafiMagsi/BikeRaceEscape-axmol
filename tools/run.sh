#!/usr/bin/env bash
# Build, install and launch the game on an iPhone, an iOS Simulator or an Android device.
# No device ids to remember: the script finds what is available and picks it, or asks you to choose.
#
#   tools/run.sh                       # asks iOS or Android, then picks / asks for the device
#   tools/run.sh ios                   # iPhone connected? simulator booted? picks it (asks if several)
#   tools/run.sh android
#   tools/run.sh ios --list            # only show the devices it can see
#   tools/run.sh ios --device "iPhone 15"      # name part or id (skips the question)
#   tools/run.sh ios --no-build        # reuse the last build (install + launch only)
#   tools/run.sh ios --app path/to/X.app       # install this .app, no build at all
#   tools/run.sh android --debug       # debug build instead of release (release is the default)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BID="com.nextfiction.dinobikeescape"
TEAM="${TEAM_ID:-XF5237ER2B}"
PLATFORM=""; WANT=""; LIST=0; BUILD=1; APP_OVERRIDE=""; ANDROID_CONFIG="Release"

while [[ $# -gt 0 ]]; do
  case "$1" in
    ios|android) PLATFORM="$1" ;;
    --device) WANT="$2"; shift ;;
    --list) LIST=1 ;;
    --no-build) BUILD=0 ;;
    --app) APP_OVERRIDE="$2"; BUILD=0; shift ;;
    --debug) ANDROID_CONFIG="Debug" ;;
    --release) ANDROID_CONFIG="Release" ;;
    -h|--help) sed -n 2,14p "${BASH_SOURCE[0]}"; exit 0 ;;
    *) echo "unknown option: $1 (try --help)" >&2; exit 1 ;;
  esac
  shift
done

if [[ -z "$PLATFORM" ]]; then
  echo "Run on:  1) iOS (iPhone / Simulator)   2) Android"
  read -r -p "Choose [1-2]: " a
  [[ "$a" == "2" ]] && PLATFORM=android || PLATFORM=ios
fi

# choose "kind|id|name|state" lines -> sets CHOSEN. Auto-picks when exactly one is ready, asks otherwise.
CHOSEN=""
choose() {
  local ready="$1" all="$2" n pick
  n=$(printf '%s\n' "$ready" | sed '/^$/d' | wc -l | tr -d ' ')
  if [[ -n "$WANT" ]]; then
    CHOSEN=$(printf '%s\n%s\n' "$ready" "$all" | sed '/^$/d' | grep -i -- "$WANT" | head -1 || true)
    [[ -n "$CHOSEN" ]] || { echo "no device matches '$WANT'" >&2; exit 1; }
    return
  fi
  if [[ "$n" == "1" ]]; then
    CHOSEN=$(printf '%s\n' "$ready" | sed '/^$/d' | head -1)
    echo "Using the only available target: $(echo "$CHOSEN" | cut -d'|' -f3)"
    return
  fi
  local list="$ready"
  if [[ "$n" == "0" ]]; then
    echo "Nothing is connected or running. Pick a target (a simulator will be started):"
    list="$all"
  else
    echo "More than one target is ready. Pick one:"
  fi
  local i=1 line
  while IFS= read -r line; do
    [[ -z "$line" ]] && continue
    printf '  %d) %-9s %s   [%s]\n' "$i" "$(echo "$line" | cut -d'|' -f1)" "$(echo "$line" | cut -d'|' -f3)" "$(echo "$line" | cut -d'|' -f4)"
    i=$((i + 1))
  done <<< "$list"
  read -r -p "Choose [1-$((i - 1))]: " pick
  CHOSEN=$(printf '%s\n' "$list" | sed '/^$/d' | sed -n "${pick}p")
  [[ -n "$CHOSEN" ]] || { echo "invalid choice" >&2; exit 1; }
}

# ---------------------------------------------------------------------------------------------------------------- iOS
ios_targets() {   # prints: ready lines, then a '---' line, then all lines
  python3 - <<'PY'
import json, subprocess, tempfile, os
ready, allt = [], []
try:
    f = tempfile.mktemp(suffix=".json")
    subprocess.run(["xcrun", "devicectl", "list", "devices", "--json-output", f], capture_output=True, timeout=60)
    for d in json.load(open(f))["result"]["devices"]:
        if d.get("hardwareProperties", {}).get("platform") != "iOS":
            continue
        up = d["connectionProperties"].get("tunnelState") in ("connected", "connecting") or d["connectionProperties"].get("pairingState") == "paired" and d["connectionProperties"].get("tunnelState") != "disconnected"
        line = "device|%s|%s (%s)|%s" % (d["identifier"], d["deviceProperties"]["name"], d["deviceProperties"].get("osVersionNumber", ""), "connected" if up else "not connected")
        allt.append(line)
        if up:
            ready.append(line)
    os.remove(f)
except Exception:
    pass
sims = json.loads(subprocess.run(["xcrun", "simctl", "list", "devices", "available", "-j"], capture_output=True, text=True).stdout)["devices"]
for rt, devs in sims.items():
    if "iOS" not in rt:
        continue
    ver = rt.split("iOS-")[-1].replace("-", ".")
    for s in devs:
        if "iPhone" not in s["name"] and "iPad" not in s["name"]:
            continue
        line = "simulator|%s|%s Simulator (%s)|%s" % (s["udid"], s["name"], ver, s["state"])
        allt.append(line)
        if s["state"] == "Booted":
            ready.append(line)
print("\n".join(ready)); print("---"); print("\n".join(allt))
PY
}

run_ios() {
  local out ready all
  out="$(ios_targets)"
  ready="$(printf '%s\n' "$out" | sed '/^---$/,$d')"
  all="$(printf '%s\n' "$out" | sed '1,/^---$/d')"
  if [[ "$LIST" == "1" ]]; then
    echo "Ready:"; printf '%s\n' "$ready" | sed '/^$/d;s/^/  /'; echo "Available:"; printf '%s\n' "$all" | sed '/^$/d;s/^/  /'; return
  fi
  choose "$ready" "$all"
  local kind id name
  kind=$(echo "$CHOSEN" | cut -d'|' -f1); id=$(echo "$CHOSEN" | cut -d'|' -f2); name=$(echo "$CHOSEN" | cut -d'|' -f3)
  echo "Target: $name ($kind)"
  local app="$APP_OVERRIDE"

  if [[ "$kind" == "device" ]]; then
    local archive="$ROOT/build-ios-device/BikeRaceEscapeAxmol.xcarchive"
    if [[ "$BUILD" == "1" ]]; then
      if [[ ! -d "$ROOT/build-ios-device/BikeRaceEscapeAxmol.xcworkspace" ]]; then
        echo "Generating the Xcode project (first run) ..."
        cmake -S "$ROOT" -B "$ROOT/build-ios-device" -G Xcode -DCMAKE_TOOLCHAIN_FILE="$ROOT/thirdparty/axmol/1k/ios.cmake" \
          -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64 -DPZ_DEVELOPMENT_TEAM="$TEAM" -DDEPLOYMENT_TARGET=15.0 \
          -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 -DCMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET=15.0
        (cd "$ROOT/build-ios-device" && pod install)
      fi
      rm -rf "$archive"
      xcodebuild -workspace "$ROOT/build-ios-device/BikeRaceEscapeAxmol.xcworkspace" -scheme BikeRaceEscapeAxmol -configuration Release \
        -destination 'generic/platform=iOS' -archivePath "$archive" -allowProvisioningUpdates archive
    fi
    [[ -n "$app" ]] || app="$archive/Products/Applications/BikeRaceEscapeAxmol.app"
    xcrun devicectl device install app --device "$id" "$app"
    xcrun devicectl device process launch --device "$id" --terminate-existing "$BID"
  else
    local sim="$ROOT/build-ios-sim"
    if [[ "$(echo "$CHOSEN" | cut -d'|' -f4)" != "Booted" ]]; then
      xcrun simctl boot "$id"; open -a Simulator
    fi
    if [[ "$BUILD" == "1" ]]; then
      if [[ ! -d "$sim/BikeRaceEscapeAxmol.xcworkspace" ]]; then
        echo "Generating the simulator project (first run) ..."
        cmake -S "$ROOT" -B "$sim" -G Xcode -DCMAKE_TOOLCHAIN_FILE="$ROOT/thirdparty/axmol/1k/ios.cmake" -DCMAKE_SYSTEM_NAME=iOS \
          -DCMAKE_OSX_ARCHITECTURES=arm64 -DSIMULATOR=TRUE -DPZ_DEVELOPMENT_TEAM="$TEAM" -DDEPLOYMENT_TARGET=15.0 -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
        (cd "$sim" && pod install)
      fi
      xcodebuild -workspace "$sim/BikeRaceEscapeAxmol.xcworkspace" -scheme BikeRaceEscapeAxmol -configuration Debug \
        -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath "$sim/dd" \
        CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO ARCHS=arm64 ONLY_ACTIVE_ARCH=YES build
    fi
    [[ -n "$app" ]] || app="$sim/dd/Build/Products/Debug-iphonesimulator/BikeRaceEscapeAxmol.app"
    xcrun simctl install "$id" "$app"
    xcrun simctl launch "$id" "$BID"
  fi
}

# ------------------------------------------------------------------------------------------------------------- Android
run_android() {
  local adb="${ADB:-$(command -v adb || true)}"
  [[ -x "$adb" ]] || adb="$HOME/Library/Android/sdk/platform-tools/adb"
  [[ -x "$adb" ]] || { echo "adb not found (install Android platform-tools)" >&2; exit 1; }
  local ready all line serial model
  ready=""; all=""
  while read -r serial state _; do
    [[ -z "$serial" || "$serial" == "List" ]] && continue
    model=$("$adb" -s "$serial" shell getprop ro.product.model 2>/dev/null | tr -d '\r' || true)
    kind="device"; [[ "$serial" == emulator-* ]] && kind="emulator"
    line="$kind|$serial|${model:-$serial}|$state"
    all="$all$line"$'\n'
    [[ "$state" == "device" ]] && ready="$ready$line"$'\n'
  done < <("$adb" devices | tail -n +2)
  if [[ "$LIST" == "1" ]]; then echo "Android targets:"; printf '%s' "$all" | sed 's/^/  /'; return; fi
  [[ -n "$all" ]] || { echo "No Android device or emulator found. Plug in a phone (USB debugging on) or start an emulator." >&2; exit 1; }
  choose "$ready" "$all"
  serial=$(echo "$CHOSEN" | cut -d'|' -f2)
  echo "Target: $(echo "$CHOSEN" | cut -d'|' -f3) ($serial)"
  local apk="$ROOT/proj.android/app/build/outputs/apk/$(echo "$ANDROID_CONFIG" | tr 'A-Z' 'a-z')/BikeRaceEscapeAxmol-$(echo "$ANDROID_CONFIG" | tr 'A-Z' 'a-z').apk"
  [[ -n "$APP_OVERRIDE" ]] && apk="$APP_OVERRIDE"
  if [[ "$BUILD" == "1" ]]; then
    (cd "$ROOT/proj.android" && ./gradlew ":BikeRaceEscapeAxmol:assemble${ANDROID_CONFIG}")
  fi
  "$adb" -s "$serial" install -r "$apk"
  "$adb" -s "$serial" shell monkey -p "$BID" -c android.intent.category.LAUNCHER 1 >/dev/null
  echo "Launched $BID on $serial"
}

case "$PLATFORM" in
  ios) run_ios ;;
  android) run_android ;;
esac
