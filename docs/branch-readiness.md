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

Held to avoid interference:

- Android app build/install/runtime validation is intentionally not run while
  another Android app is being tested on this machine.

Blocked next by runtime/hardware:

- Android SDK AVD acceleration requires `/dev/kvm`, currently blocked by BIOS
  virtualization state.
- ARCore, camera, media projection, overlay windows, sensors, glasses bridge,
  and physical input require runtime validation and/or physical hardware.

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

Blocked next by runtime/hardware:

- Windows 11 native runtime validation requires a Windows host or configured VM
  image.
- Win32/WinUI graphics, drivers, camera/sensors, glasses bridge, and physical
  input require a Windows runtime and/or physical hardware.
