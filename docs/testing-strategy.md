# Testing Strategy

## Test Levels

- Schema tests: validate every shared API and scene message.
- Core tests: validate overlay state transitions and error behavior.
- Adapter tests: validate platform adapter lifecycle and capability reporting.
- Visual tests: compare deterministic scenes across renderers.
- Input replay tests: feed shared action fixtures and verify state transitions.
- Hardware smoke tests: verify real device connection, presentation, and teardown.

## No-Hardware Requirement

Every branch must keep a no-hardware path. Hardware access can add confidence,
but it must not be required to run the core contract tests.

## Branch Gate

Before merging platform work:

- Run shared tests.
- Run platform adapter tests.
- Capture or regenerate visual fixture evidence.
- Document hardware used, or explicitly state that the branch was verified by
  the no-hardware simulator only.
