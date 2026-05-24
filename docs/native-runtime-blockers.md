# Native Runtime Blockers

Checked on 2026-05-24 from the Debian 13 development host.

These blockers do not stop the repo-native no-hardware emulators. They affect
native OS runtime validation only.

## Android

Status: partially available.

- Android SDK tooling is installed.
- Waydroid is running and boot-complete.
- Android SDK AVD acceleration is blocked because `/dev/kvm` is missing.
- Android app build/install/runtime validation is currently held to avoid
  interfering with another Android app test running on this machine.

Required external action:

- Enable Intel VT-x/virtualization in firmware/BIOS, reboot, and recheck
  `/dev/kvm` before treating Android SDK AVD runs as reliable.

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
