# Shared Experience Contract

Every hardware branch must preserve the same externally visible experience.
Platform adapters can differ internally, but the UI, API, visualization behavior,
and error semantics must remain equivalent.

## UI Contract

- Commands use the same names, labels, states, and confirmations across
  platforms.
- Controls expose the same enable/disable logic for unavailable hardware
  features.
- Platform-specific permission prompts can exist, but the app-level recovery
  flow must return to the same state names.
- Accessibility names, focus order, and text alternatives must be shared unless
  a platform accessibility framework requires a documented variation.

## API Contract

- API messages use the shared envelope in
  `src/contracts/overlay-message.schema.json`.
- Platform branches may add adapter diagnostics, but they must not require
  platform-only fields for shared commands.
- Timestamps are UTC ISO 8601 strings.
- Coordinates are normalized unless a command explicitly names a physical unit.
- Errors use stable shared codes, with optional platform detail attached under
  `detail`.

## Visualization Contract

- Overlay elements render in deterministic z-order.
- Colors use sRGB values in shared data; platform adapters handle display color
  conversion.
- Text layout must preserve declared alignment, wrapping, and bounds.
- Motion timing uses shared durations and easing names.
- Missing fonts fall back through a shared logical font map.
- Branches must capture a visual fixture before claiming parity.

## Input Contract

- Input events are represented as shared actions, not raw platform events.
- Supported actions start with `select`, `back`, `dismiss`, `drag`, `pinch`,
  `scroll`, and `text`.
- Gesture recognition can be platform-specific, but emitted action payloads must
  match shared test fixtures.

## Capability Contract

Each adapter must publish capabilities at startup:

- display mode and resolution class
- refresh rate or timing class
- input methods
- permission state
- sensor availability
- hardware identifier when available
- unsupported shared features with reason codes

The shared UI must degrade from capabilities, not from platform names.
