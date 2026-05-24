# Emulator Verification

The repository provides no-hardware platform emulators for Linux, Android, iOS,
and Windows. These are repo-native emulators that exercise the shared API,
adapter lifecycle, scene state machine, renderer, diagnostics, and transcript
generation without physical AR/XR hardware.

## Commands

- `npm run emulate` - run all platform emulators.
- `npm run check:host-emulators` - report native host emulator availability and
  blockers.
- `npm run verify:platform-scaffolds` - validate native/no-hardware platform
  scaffolds without launching Android runtime resources.
- `npm run verify` - run validation, tests, frame comparison, frame rendering,
  all platform emulators, and host emulator checks.
- `npm run serve:api` - start the local HTTP API transport for manual command
  testing.

## Outputs

Generated outputs are written under `build/` and are intentionally ignored by
git:

- `build/frames/*.svg`
- `build/frames/manifest.json`
- `build/emulators/<platform>/capabilities.json`
- `build/emulators/<platform>/diagnostics.json`
- `build/emulators/<platform>/transcript.json`
- `build/emulators/<platform>/*.svg`

## Native Runtime Status

Current host checks show:

- Linux: repo-native emulator runs locally.
- Android: Android SDK tooling and Waydroid exist. Waydroid is running and
  boot-complete. Android SDK AVD acceleration is blocked until `/dev/kvm` is
  available.
- iOS: Apple iOS Simulator is not available on this Debian host.
- Windows: Wine and QEMU are installed, but no Windows 11 VM image is configured
  in this repo. The Windows branch uses the repo-native emulator until a VM or
  Windows host is available.

## Hardware Boundary

Physical hardware is still required for real glasses or bridge discovery,
presentation timing, sensors, physical input, vendor SDK behavior, and native
permission or driver flows that cannot be simulated.
