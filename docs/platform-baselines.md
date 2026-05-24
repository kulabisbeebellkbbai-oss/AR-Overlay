# Platform Baselines

Checked on 2026-05-24.

## Linux

Baseline: platform-agnostic Linux first, with Debian GNU/Linux 13.5 on this
workstation as the concrete verification host when a specific direction is
required.

Local host facts:

- OS: Debian GNU/Linux 13.5 (`trixie`)
- Kernel: `6.12.86+deb13-amd64`
- Architecture: `x86_64`

Implementation direction:

- Prefer compositor/display abstractions that can work across Wayland and X11.
- Keep hardware adapters isolated behind capability probes.
- Use Debian packaging and local scripts only as the first verification path,
  not as a hard product dependency.

## Android

Baseline: Android 15, API level 35.

Current official Android 15 behavior notes that matter for this project:

- Android 15 applies behavior changes to all apps running on the platform.
- Apps with `targetSdkVersion` lower than 24 cannot be installed on Android 15.
- Apps targeting Android 15/API 35 get edge-to-edge enforcement and several
  foreground-service/background-launch behavior changes.

Implementation direction:

- Target Android 15/API 35 for new development.
- Treat overlay/window behavior, media projection, background work, and local
  network discovery as explicit adapter concerns.
- Keep shared API and rendering fixtures independent of Android UI toolkit
  choices.

References:

- https://developer.android.com/about/versions/15/behavior-changes-all
- https://developer.android.com/about/versions/15/reference/compat-framework-changes

## iOS

Baseline: current iOS 26 SDK generation, with backwards compatibility preserved
without sacrificing required functionality.

Current official Apple support notes that matter for this project:

- Current Xcode support lists iOS 26.5 SDKs and deployment targets covering iOS
  15 through iOS 26.5.
- iOS 26 compatibility reaches back to iPhone 11 and iPhone SE 2nd generation.
- ARKit/device capability must be checked at runtime rather than assumed for all
  iOS devices.

Implementation direction:

- Use iOS 15 as the default minimum deployment target while this remains
  compatible with required functionality.
- Gate newer APIs with availability checks.
- Treat camera, sensor, ARKit, external display, and glasses bridge access as
  capability-driven adapter features.

References:

- https://developer.apple.com/support/xcode/
- https://support.apple.com/en-sg/123705
- https://developer.apple.com/documentation/ARKit/verifying-device-support-and-user-permission

## Windows

Baseline: current Windows 11 general availability state.

Current official Microsoft notes that matter for this project:

- Windows 11 follows an annual feature-update cadence with monthly cumulative
  security updates.
- Current Windows 11 servicing information includes 26H1 for new devices and
  25H2/24H2 for existing general-availability servicing.
- Windows 11 25H2 OS build 26200 had a May 2026 hotpatch build listed as
  26200.8390 at the time of this check.

Implementation direction:

- Develop for current Windows 11, with 25H2/24H2 compatibility assumptions until
  hardware requires 26H1-only features.
- Keep WinUI, graphics, device SDK, and driver integration behind adapter
  boundaries.
- Use capability probes instead of build-number checks wherever possible.

References:

- https://learn.microsoft.com/en-us/windows/release-health/windows11-release-information
