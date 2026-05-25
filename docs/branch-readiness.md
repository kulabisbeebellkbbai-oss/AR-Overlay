# Branch Readiness

Checked on 2026-05-24.

All four hardware branches now include the shared no-hardware core, platform
emulator coverage, and native/no-hardware scaffold coverage. The repository is
at the point where remaining meaningful work depends on physical AR/XR hardware
or unavailable native OS runtime hosts.

## Linux

Ready without hardware:

- Shared API, scene, input, simulator, and emulator verification.
- Native CMake smoke binary at `platforms/linux`.
- Local build and smoke execution through `npm run verify:platform-scaffolds`.

Blocked next by hardware:

- Real glasses/display bridge discovery.
- Wayland/X11 compositor presentation timing.
- Physical input, sensors, and device-specific graphics behavior.

## Android

Ready without hardware:

- Shared API, scene, input, simulator, and emulator verification.
- Android Gradle app scaffold under `android`.
- Static Android scaffold validation through `npm run verify:platform-scaffolds`.
- Android SDK tooling, Gradle, ADB, Android Emulator, Android 36.1 image, and
  the `codex_api36` AVD are present on this host.
- `/dev/kvm` is available, so Android SDK emulator acceleration is available.
- Android emulator start is available through `npm run android:start-emulator`.
- Android build/install/launch/focus/screenshot validation is available through
  `npm run android:emulator-smoke`.
- Full emulator validation is available through `npm run android:emulator-test`.
- Emulator validation passed on `emulator-5554` with Android 16/API 36,
  including visual assertions and back-input state transition.

Blocked next by runtime/hardware:

- ARCore, camera, media projection, overlay windows, sensors, glasses bridge,
  and physical input require emulator/device runtime validation and/or physical
  hardware.

## iOS

Ready without hardware:

- Shared API, scene, input, simulator, and emulator verification.
- Swift/SwiftUI scaffold under `platforms/ios`.
- Static iOS scaffold validation through `npm run verify:platform-scaffolds`.
- iOS deployment target remains 15.0 with ARKit availability checks.

Blocked next by runtime/hardware:

- Native build and simulator validation require macOS with Xcode.
- ARKit, camera, sensors, glasses bridge, and physical input require Apple
  simulator support and/or physical hardware.

## Windows

Ready without hardware:

- Shared API, scene, input, simulator, and emulator verification.
- C++/CMake smoke scaffold under `platforms/windows`.
- Local host-check build and smoke execution through
  `npm run verify:platform-scaffolds`.

Hardware bring-up target:

- First connected test hardware is XReal 1S on Windows 11.
- User-reported state: XReal 1S glasses are connected to the Windows computer
  and working properly.
- Bring-up procedure is documented in `docs/hardware/xreal-1s-windows11.md`.

Blocked next by hands-on Windows hardware testing:

- Run `scripts/windows-xreal-smoke.ps1` on the Windows 11 host.
- Validate overlay placement on the glasses display.
- Measure real presentation timing and record any display mode/scaling issues.
- Validate physical input, sensors, vendor tooling, or driver behavior if
  exposed by the XReal 1S path.
