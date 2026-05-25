#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

export ANDROID_HOME="${ANDROID_HOME:-$HOME/Android/Sdk}"
export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$ANDROID_HOME}"
export ANDROID_NDK_HOME="${ANDROID_NDK_HOME:-$ANDROID_HOME/ndk/29.0.14206865}"
export GRADLE_HOME="${GRADLE_HOME:-$HOME/.local/opt/gradle-9.5.1}"
export PATH="$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$GRADLE_HOME/bin:$PATH"

OUT_DIR="hardware-results/android-emulator"
mkdir -p "$OUT_DIR"

{
  echo "# Android Emulator Smoke"
  echo
  echo "Checked: $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  echo "Branch: $(git rev-parse --abbrev-ref HEAD)"
  echo "Commit: $(git rev-parse --short HEAD)"
  echo
  echo "## Tooling"
  java -version 2>&1 | sed 's/^/- /'
  gradle --version | sed -n '1,8p' | sed 's/^/- /'
  adb version | sed 's/^/- /'
  emulator -version 2>&1 | head -n 2 | sed 's/^/- /'
  echo
  echo "## Build"
} > "$OUT_DIR/summary.md"

gradle --no-daemon :android:assembleDebug | tee "$OUT_DIR/gradle-assemble-debug.log"
APK="android/build/outputs/apk/debug/android-debug.apk"
if [[ ! -f "$APK" ]]; then
  echo "APK not found at $APK" | tee -a "$OUT_DIR/summary.md"
  exit 1
fi

{
  echo "- Built APK: $APK"
  echo "- APK size bytes: $(wc -c < "$APK")"
  echo
  echo "## ADB Devices"
} >> "$OUT_DIR/summary.md"

adb devices -l | tee "$OUT_DIR/adb-devices.txt"
cat "$OUT_DIR/adb-devices.txt" >> "$OUT_DIR/summary.md"

DEVICE="$(awk 'NR > 1 && $2 == "device" { print $1; exit }' "$OUT_DIR/adb-devices.txt")"
if [[ -z "$DEVICE" ]]; then
  {
    echo
    echo "## Runtime"
    echo
    echo "No ADB device is visible. Build validation completed; install/launch was skipped."
  } >> "$OUT_DIR/summary.md"
  exit 0
fi

{
  echo
  echo "## Runtime"
  echo
  echo "Installing and launching on ADB device $DEVICE."
} >> "$OUT_DIR/summary.md"

"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" install -r "$APK" | tee "$OUT_DIR/adb-install.txt"
"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" shell monkey -p com.kulabisbeebellkbbai.aroverlay 1 | tee "$OUT_DIR/adb-launch.txt"
sleep 2
"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" shell pidof com.kulabisbeebellkbbai.aroverlay | tee "$OUT_DIR/adb-pidof.txt" || true
"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" shell getprop ro.build.version.release > "$OUT_DIR/android-version.txt"
"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" shell getprop ro.build.version.sdk >> "$OUT_DIR/android-version.txt"
"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" shell dumpsys window | grep -E "mCurrentFocus|mFocusedApp" > "$OUT_DIR/window-focus.txt" || true
"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" shell dumpsys activity activities \
  | grep -E "ResumedActivity|topResumedActivity|Hist" \
  | head -n 40 > "$OUT_DIR/activity-focus.txt" || true
"$ANDROID_HOME/platform-tools/adb" -s "$DEVICE" exec-out screencap -p > "$OUT_DIR/screen.png"
node scripts/assert-android-emulator-results.js --mode=initial

{
  echo "- Target device: $DEVICE"
  echo "- Install result:"
  sed 's/^/  /' "$OUT_DIR/adb-install.txt"
  echo "- Launch result:"
  sed 's/^/  /' "$OUT_DIR/adb-launch.txt"
  echo "- PID:"
  sed 's/^/  /' "$OUT_DIR/adb-pidof.txt"
  echo "- Android version/API:"
  sed 's/^/  /' "$OUT_DIR/android-version.txt"
  echo "- Window focus:"
  sed 's/^/  /' "$OUT_DIR/window-focus.txt"
  echo "- Activity focus:"
  sed 's/^/  /' "$OUT_DIR/activity-focus.txt"
  echo "- Screenshot: $OUT_DIR/screen.png"
} >> "$OUT_DIR/summary.md"
