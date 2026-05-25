# Android Hardware Branch

## Branch Goal

Build the Android AR Overlay adapter against Android 15 behavior while
preserving the shared UI, API, visualization, and input contract.

## Baseline

- Primary target: Android 15, API level 35.
- Minimum install reality on Android 15: apps targeting lower than SDK 24 are
  blocked from installation.
- New development should target API 35 unless a hardware SDK proves otherwise.

## Adapter Direction

- Keep Android UI toolkit choices behind shared rendering and input contracts.
- Treat edge-to-edge behavior, permission prompts, display cutouts, foreground
  services, media projection, and local network discovery as adapter concerns.
- Report device and OS capabilities at startup instead of branching shared code
  by Android version.
- Keep glasses SDK bindings optional until the first target device is selected.

## Initial Work Items

1. Use `createPlatformAdapter("android")` for the current no-hardware
   capability probe.
2. Use `npm run render:fixtures` for the current no-hardware renderer path.
3. Use the shared command processor for API message validation before platform
   command execution.
4. Use shared input fixtures as the target shape for Android event translation.
5. Add smoke tests for Android 15 behavior changes that affect overlays,
   background work, and projection.

## No-Hardware Verification

Run `npm run verify` on this branch before hardware or Android device work
begins.

Run `npm run android:emulator-test` when the Android emulator testing slot is
available. This starts or reuses `codex_api36`, installs the Android APK,
validates foreground focus, asserts the rendered fixture screenshot, sends the
Android back key, and verifies the shared `ready` state through the
accessibility tree.

## Compatibility Notes

- Use runtime permission and API availability checks instead of assuming
  platform features.
- Keep older-device support where it does not weaken required AR/XR overlay
  behavior.
- Do not let Android-specific lifecycle states leak into shared API responses;
  map them to shared session and error states.
