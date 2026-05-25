# Native Runtime Blockers

Checked on 2026-05-24 from the Debian 13 development host.

These blockers do not stop the repo-native no-hardware emulators. They affect
native OS runtime validation only.

## Android

Status: available for emulator build/install/launch validation.

- Android SDK tooling is installed.
- Android Emulator, ADB, Gradle, API 36.1 platform/image, and the `codex_api36`
  AVD are installed.
- `/dev/kvm` is present, so Android SDK emulator acceleration is available.
- Waydroid is also installed and has a running session on this host.
- Android runtime validation is available through
  `npm run android:start-emulator` followed by `npm run android:emulator-test`.
- Build/install/launch/focus/screenshot and back-input state validation passed
  on `emulator-5554` with Android 16/API 36.

Current blocker:

- Physical AR/XR Android hardware remains required for glasses bridge,
  real-display, camera, sensor, and physical input validation.

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
