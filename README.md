# AR Overlay

AR Overlay is a cross-platform facility for displaying content on AR/XR glasses.
The project will keep platform-specific hardware integration isolated while
sharing display, content, and overlay concepts across targets.

## Hardware Tracks

The repository starts from `main` and uses four long-lived hardware branches:

- `linux`
- `android`
- `ios`
- `windows`

Keep shared architecture, protocol, asset, and documentation changes on `main`
unless they are specific to one hardware track.

## Shared Contract

All hardware branches must preserve the shared UI, API, visualization, input,
and capability behavior documented in `docs/shared-experience-contract.md`.
Platform branches may differ internally, but externally visible behavior starts
from the same shared schema and fixtures.

## Planning Documents

- `docs/project-plan.md` - phased project plan and manager queue
- `docs/platform-baselines.md` - current platform assumptions
- `docs/implementation-readiness.md` - code boundaries to create next
- `docs/testing-strategy.md` - parity and no-hardware test expectations
- `src/contracts/overlay-message.schema.json` - initial shared message envelope

## Repository Layout

- `src/` - shared source and platform entry points as they are added
- `tests/` - tests that mirror source behavior where practical
- `assets/` - visual, calibration, and sample overlay assets
- `docs/` - design notes, hardware notes, and contributor documentation

Local agent metadata belongs in `.agents/` and `.codex/`; those directories are
excluded from git in this checkout.

## Development

The first functional slice uses dependency-free Node.js ES modules.

- `npm run validate:fixtures` - validate shared scene fixtures.
- `npm test` - run schema, state-machine, simulator, and adapter tests.
- `npm run render:fixtures` - render deterministic no-hardware SVG frames into
  `build/frames/`.
- `npm run compare:frames` - compare generated frame hashes with checked-in
  expected hashes.
- `npm run emulate` - run Linux, Android, iOS, and Windows no-hardware platform
  emulators and write transcripts into `build/emulators/`.
- `npm run check:host-emulators` - report native host emulator availability and
  blockers.
- `npm run serve:api` - start the local HTTP API transport.
- `npm run verify` - run fixture validation, tests, frame comparison, frame
  rendering, platform emulators, and host emulator checks.

## Functional Baseline

The current implementation supports:

- shared API message helpers and validation
- shared scene validation and normalization
- overlay session state transitions
- transport-independent command processing
- HTTP API transport for no-hardware clients
- no-hardware platform capability adapters
- deterministic SVG rendering for visual fixtures
- expected frame-hash comparison for deterministic visual parity
- Linux, Android, iOS, and Windows platform emulator transcripts
- input and scene fixtures for branch parity testing
