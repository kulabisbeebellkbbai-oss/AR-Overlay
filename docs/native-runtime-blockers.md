# Native Runtime Blockers

Checked on 2026-05-24 from the Debian 13 development host.

These blockers do not stop the repo-native no-hardware emulators. They affect
native OS runtime validation only.

## Android

Status: available for guarded emulator build/install validation when an ADB
device is visible.

- Android SDK tooling is installed.
- Android Emulator, ADB, Gradle, API 36.1 platform/image, and the `codex_api36`
  AVD are installed.
- `/dev/kvm` is present, so Android SDK emulator acceleration is available.
- Waydroid is also installed and has a running session on this host.
- Android build/install/runtime validation is guarded by
  `npm run android:emulator-smoke`, which does not start, stop, or reset
  emulator state.

Current blocker:

- A visible ADB device is required for install/launch validation. If `adb
  devices -l` is empty, build validation can still complete but runtime launch
  evidence cannot be collected from this shell.

## iOS

Status: blocked on this host.

- Apple iOS Simulator requires macOS and Xcode.
- This Debian host does not provide `xcrun` or the Apple simulator runtime.

Required external action:

- Use a macOS host with Xcode for native iOS simulator validation.

## Windows

Status: ready for first hardware bring-up on a Windows 11 host.

- XReal 1S is the first connected AR glasses target.
- The glasses are connected to the Windows 11 computer and working properly,
  according to user report.
- QEMU and Wine remain available on the Debian host for limited local checks,
  but the meaningful next validation is on the real Windows 11 hardware host.

Required next action:

- Run the Windows hardware smoke procedure in
  `docs/hardware/xreal-1s-windows11.md`.
