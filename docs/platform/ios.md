# iOS Hardware Branch

## Branch Goal

Build the iOS AR Overlay adapter against the current iOS SDK generation while
keeping the broadest practical backwards compatibility.

## Baseline

- Current SDK line: iOS 26 generation.
- Initial minimum deployment target: iOS 15, as long as required overlay
  functionality remains available.
- Device support must be capability-driven because ARKit, camera, sensor,
  external display, and glasses bridge features vary by model and OS version.

## Adapter Direction

- Use Swift and Apple platform APIs when implementation begins unless the shared
  stack choice requires another binding.
- Gate newer APIs with availability checks.
- Keep ARKit, RealityKit, camera, external display, and accessory integration
  behind adapter interfaces.
- Preserve shared API and visual behavior even when native permission flows or
  lifecycle states differ.

## Initial Work Items

1. Create an iOS capability probe for OS, device, display, camera, ARKit, input,
   permissions, and available glasses bridge APIs.
2. Create a no-hardware simulator or preview path that consumes shared visual
   fixtures.
3. Add shared API message validation before native command execution.
4. Add input action translation from touch, pointer, controller, or glasses
   bridge events to shared actions.
5. Add availability tests for iOS 15 through the current SDK target where tools
   allow.

## Compatibility Notes

- Prefer iOS 15 compatibility until a required AR/XR capability demands a newer
  deployment target.
- Newer APIs must fail gracefully through capability reporting.
- Do not expose Apple-specific lifecycle or permission vocabulary in shared API
  responses; map it to shared session and error states.
