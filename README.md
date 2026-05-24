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

## Repository Layout

- `src/` - shared source and platform entry points as they are added
- `tests/` - tests that mirror source behavior where practical
- `assets/` - visual, calibration, and sample overlay assets
- `docs/` - design notes, hardware notes, and contributor documentation

Local agent metadata belongs in `.agents/` and `.codex/`; those directories are
excluded from git in this checkout.

## Development

No build system is selected yet. Add install, run, test, and lint commands to
`AGENTS.md` and this README when the first implementation stack is chosen.
