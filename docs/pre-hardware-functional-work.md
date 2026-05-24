# Pre-Hardware Functional Work

This document tracks work that can be completed before physical AR/XR glasses or
display bridges are required.

## Completed Baseline

- Selected the initial shared stack: dependency-free Node.js ES modules.
- Added shared package boundaries under `src/api`, `src/core`,
  `src/adapter-kit`, and `src/simulator`.
- Added shared schemas for messages, scenes, commands, responses, events,
  capabilities, and errors.
- Added shared scene fixtures for text, image, z-ordering, and motion metadata.
- Added shared input fixtures for replayable actions.
- Implemented shared scene validation and normalization.
- Implemented overlay session state transitions.
- Implemented transport-independent command processing.
- Implemented an HTTP API transport for no-hardware command testing.
- Implemented no-hardware platform adapters for Linux, Android, iOS, and
  Windows.
- Implemented simulated adapter lifecycle methods for display session creation,
  scene presentation, scene updates, input translation, permission recovery,
  diagnostics, and shutdown.
- Implemented deterministic SVG frame rendering.
- Added frame hash manifest generation and checked-in expected frame hashes.
- Added tests for schemas, input replay, command processing, state transitions,
  HTTP transport, platform emulators, simulator output, and adapter
  capabilities.
- Added platform emulator transcripts and diagnostics for Linux, Android, iOS,
  and Windows.
- Added native host runtime blocker reporting for Android, iOS, and Windows.
- Added native/no-hardware platform scaffolds for Linux, Android, iOS, and
  Windows.
- Added scaffold verification and a guarded Android emulator smoke path that
  builds the APK and only installs/launches when an ADB device is already
  visible.

## Completion State

The project is now at the no-hardware boundary for all four hardware branches.
Remaining work requires native runtime access that is unavailable on this host,
a visible emulator/device runtime target, or physical AR/XR hardware.
- Added a GitHub Actions no-hardware verification workflow.

## Additional Non-Blocking Pre-Hardware Enhancements

- Add a real transport layer around the existing command processor, such as
  WebSocket or platform-native IPC.
- Add more complex layout primitives, including groups, anchors, clipping,
  opacity inheritance, and text wrapping rules.
- Add a shared logical font map and fallback tests.
- Add permission-denied and recovery scenario fixtures.
- Add diagnostics export fixtures and snapshots.
- Add per-platform packaging scaffolds once the implementation language for
  native adapters is selected.
- Add branch protection expectations once CI has run successfully on GitHub.

## Hardware Required Later

Physical hardware becomes required for:

- real glasses or bridge discovery
- real display connection and presentation timing
- camera, sensor, and head-pose validation
- physical input delivery
- vendor SDK behavior
- driver, entitlement, or permission flows that cannot be simulated
- comfort, latency, thermal, and battery evaluation
