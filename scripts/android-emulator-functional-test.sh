#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

export ANDROID_HOME="${ANDROID_HOME:-$HOME/Android/Sdk}"
export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$ANDROID_HOME}"
export GRADLE_HOME="${GRADLE_HOME:-$HOME/.local/opt/gradle-9.5.1}"
export PATH="$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$GRADLE_HOME/bin:$PATH"

OUT_DIR="hardware-results/android-emulator"
mkdir -p "$OUT_DIR"

npm run android:start-emulator
npm run android:emulator-smoke
node scripts/assert-android-emulator-results.js --mode=initial

DEVICE="$(awk 'NR > 1 && $2 == "device" { print $1; exit }' "$OUT_DIR/adb-devices.txt")"
if [[ -z "$DEVICE" ]]; then
  echo "No ADB device found after emulator smoke."
  exit 1
fi

adb -s "$DEVICE" shell input keyevent KEYCODE_BACK
sleep 1
adb -s "$DEVICE" shell dumpsys window | grep -E "mCurrentFocus|mFocusedApp" > "$OUT_DIR/back-window-focus.txt" || true
adb -s "$DEVICE" shell uiautomator dump /sdcard/ar-overlay-window.xml >/dev/null
adb -s "$DEVICE" exec-out cat /sdcard/ar-overlay-window.xml > "$OUT_DIR/back-window.xml"
adb -s "$DEVICE" exec-out screencap -p > "$OUT_DIR/back-screen.png"

if ! grep -q "ar-overlay-state:ready" "$OUT_DIR/back-window.xml"; then
  echo "Back input did not expose shared ready state in accessibility tree."
  exit 1
fi

node scripts/assert-android-emulator-results.js \
  --mode=dismissed \
  --screenshot="$OUT_DIR/back-screen.png" \
  --focus="$OUT_DIR/back-window-focus.txt"

{
  echo
  echo "## Input"
  echo
  echo "- Back key translated to shared ready state."
  echo "- Back focus evidence: $OUT_DIR/back-window-focus.txt"
  echo "- Back accessibility evidence: $OUT_DIR/back-window.xml"
  echo "- Back screenshot: $OUT_DIR/back-screen.png"
} >> "$OUT_DIR/summary.md"
