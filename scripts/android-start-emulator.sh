#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

export ANDROID_HOME="${ANDROID_HOME:-$HOME/Android/Sdk}"
export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$ANDROID_HOME}"
export GRADLE_HOME="${GRADLE_HOME:-$HOME/.local/opt/gradle-9.5.1}"
export PATH="$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools:$ANDROID_HOME/emulator:$GRADLE_HOME/bin:$PATH"

AVD_NAME="${AVD_NAME:-codex_api36}"
OUT_DIR="build/android-emulator"
mkdir -p "$OUT_DIR"

if adb devices | awk 'NR > 1 && $2 == "device" { found = 1 } END { exit found ? 0 : 1 }'; then
  echo "ADB device already available."
  adb devices -l
  exit 0
fi

LOG="$OUT_DIR/$AVD_NAME.log"
PID_FILE="$OUT_DIR/$AVD_NAME.pid"
rm -f "$LOG"

setsid emulator -avd "$AVD_NAME" \
  -no-window \
  -no-audio \
  -no-snapshot \
  -no-boot-anim \
  -gpu swiftshader_indirect \
  -memory 2048 \
  -netdelay none \
  -netspeed full \
  -no-metrics \
  > "$LOG" 2>&1 < /dev/null &
echo "$!" > "$PID_FILE"

echo "Started $AVD_NAME with pid $(cat "$PID_FILE")."

for attempt in $(seq 1 120); do
  device="$(adb devices | awk 'NR > 1 && $2 == "device" { print $1; exit }')"
  if [[ -n "$device" ]]; then
    boot="$(adb -s "$device" shell getprop sys.boot_completed 2>/dev/null | tr -d '\r')"
    echo "device=$device boot=$boot attempt=$attempt"
    if [[ "$boot" == "1" ]]; then
      exit 0
    fi
  else
    echo "waiting-for-adb attempt=$attempt"
  fi
  sleep 2
done

echo "Emulator did not finish booting. Recent log:"
tail -n 120 "$LOG"
exit 1
