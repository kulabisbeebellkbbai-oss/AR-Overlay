# Project Plan

## Goal

Build a cross-platform AR/XR overlay facility that presents the same content,
control surface, API semantics, and visual behavior on Linux, Android, iOS, and
Windows hardware tracks.

## Operating Model

- `main` owns shared product decisions, contracts, schemas, documentation, and
  test expectations.
- `linux`, `android`, `ios`, and `windows` own platform-specific adapters,
  packaging, permissions, display integration, and hardware notes.
- Platform branches must not redefine user-visible behavior. If behavior needs
  to change, update the shared contract on `main` first, then merge it into each
  platform branch.

## Phase 0: Definition And Contracts

Status: started.

- Define overlay primitives, API envelopes, visual invariants, and platform
  readiness expectations.
- Document current platform baselines and compatibility targets.
- Prepare branch-specific docs so implementation can start without reopening
  basic architecture questions.

## Phase 1: Shared Functional Core

Status: queued.

- Choose the first implementation language and build tooling.
- Implement a platform-neutral overlay scene model.
- Add serialization/deserialization for shared API messages.
- Add a no-hardware renderer or simulator that can validate layout, ordering,
  color, sizing, and state transitions.
- Add contract tests that every platform branch must run.

## Phase 2: Platform Adapters

Status: queued.

- Linux: build the first local host implementation and simulator path.
- Android: build against Android 15/API 35 behavior while preserving compatible
  runtime checks for older supported devices where possible.
- iOS: build against current Xcode/iOS SDK support while preserving deployment
  compatibility as far back as iOS 15 where functionality remains intact.
- Windows: build for current Windows 11, with adapter boundaries ready for
  graphics/display APIs and device-specific SDKs.

## Phase 3: Hardware Bring-Up

Status: queued.

- Select the first target AR/XR glasses or display bridge for each platform.
- Add per-platform hardware capability manifests.
- Validate display connection, presentation timing, input delivery, and teardown.
- Record device-specific deviations as adapter constraints, not shared behavior.

## Phase 4: Cross-Platform Verification

Status: queued.

- Run API contract tests on every platform branch.
- Run screenshot or frame-capture comparisons against shared visual fixtures.
- Run input event replay tests against the same scenario files.
- Require parity notes in every platform pull request.

## Near-Term Manager Queue

1. Select the shared implementation stack.
2. Add the first shared scene model and API message types.
3. Add a no-hardware visual simulator.
4. Add branch CI or local scripts for contract verification.
5. Start Linux adapter implementation first because this Debian host can verify
   it locally.
