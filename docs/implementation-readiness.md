# Implementation Readiness

This repository now has a dependency-free Node.js ES module baseline for the
first no-hardware functional slice.

## Shared Packages

- `src/core`: overlay scene model, state machine, validation, and shared errors.
- `src/api`: command envelope and transport-independent message helpers.
- `fixtures/scenes`: canonical scenes for screenshot and frame comparison.
- `fixtures/inputs`: canonical input event replay files.
- `src/adapter-kit`: interfaces and no-hardware adapters that platform branches
  implement.
- `src/simulator`: deterministic no-hardware SVG renderer.

## Adapter Responsibilities

Each platform adapter must implement:

- capability probe
- display/session lifecycle
- overlay element creation, update, ordering, and removal
- input action translation
- permission recovery
- diagnostics export
- graceful shutdown

## First Functional Slice

The first useful vertical slice is implemented:

1. Load a shared scene fixture.
2. Validate the fixture against the shared schema.
3. Render a static overlay layer in a no-hardware simulator.
4. Emit a capability report.
5. Accept a shared `dismiss` or `select` input action.
6. Produce a deterministic screenshot or frame output.

Run it with `npm run verify`.

## Done Criteria For Platform Parity

- Shared schema tests pass.
- Shared visual fixture renders without unsupported elements.
- Input replay produces the same shared state transitions.
- Unsupported hardware features are reported through capability reasons.
- Platform-specific docs identify every intentional deviation.
