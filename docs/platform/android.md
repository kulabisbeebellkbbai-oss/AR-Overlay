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

1. Create an Android capability probe for OS, display, sensors, input, and
   permission state.
2. Create a no-hardware activity or renderer path that consumes shared visual
   fixtures.
3. Add shared API message validation before platform command execution.
4. Add input action translation from Android events to shared actions.
5. Add smoke tests for Android 15 behavior changes that affect overlays,
   background work, and projection.

## Compatibility Notes

- Use runtime permission and API availability checks instead of assuming
  platform features.
- Keep older-device support where it does not weaken required AR/XR overlay
  behavior.
- Do not let Android-specific lifecycle states leak into shared API responses;
  map them to shared session and error states.
