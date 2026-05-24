# Native Runtime Blockers

Checked on 2026-05-24 from the Debian 13 development host.

These blockers do not stop the repo-native no-hardware emulators. They affect
native OS runtime validation only.

## Android

Status: partially available.

- Android SDK tooling is installed.
- Waydroid is running and boot-complete.
- Android SDK AVD acceleration is blocked because `/dev/kvm` is missing.

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

Status: partially available.

- QEMU and Wine are available on this host.
- No Windows 11 VM image or packaged Windows build is configured in this repo.

Required external action:

- Add a Windows 11 VM image and install/run instructions, or validate on a
  Windows 11 host once the native Windows package exists.
