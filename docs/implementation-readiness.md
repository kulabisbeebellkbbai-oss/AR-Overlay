# Implementation Readiness

This repository is ready for functional code after the shared stack is selected.
The following boundaries should be created first.

## Shared Packages

- `core`: overlay scene model, state machine, validation, and shared errors.
- `api`: command envelope, transport-independent request/response contracts, and
  event stream contracts.
- `visual-fixtures`: canonical scenes for screenshot and frame comparison.
- `input-fixtures`: canonical input event replay files.
- `adapter-kit`: interfaces that platform branches implement.

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

The first useful vertical slice should be:

1. Load a shared scene fixture.
2. Validate the fixture against the shared schema.
3. Render a static overlay layer in a no-hardware simulator.
4. Emit a capability report.
5. Accept a shared `dismiss` or `select` input action.
6. Produce a deterministic screenshot or frame output.

## Done Criteria For Platform Parity

- Shared schema tests pass.
- Shared visual fixture renders without unsupported elements.
- Input replay produces the same shared state transitions.
- Unsupported hardware features are reported through capability reasons.
- Platform-specific docs identify every intentional deviation.
