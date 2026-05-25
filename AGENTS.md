# Repository Guidelines

## Project Structure & Module Organization

Keep source code in `src/`, tests in `tests/`, assets in `assets/`, and contributor notes in `docs/` as the project grows. Keep `.agents/` and `.codex/` for local agent metadata. Shared contracts belong on `main`; platform-specific adapter work belongs on `linux`, `android`, `ios`, or `windows`.

## Build, Test, and Development Commands

The first functional slice uses dependency-free Node.js ES modules.

- `npm run validate:fixtures` - validate shared scene fixtures.
- `npm test` - run schema, state-machine, simulator, and adapter tests.
- `npm run render:fixtures` - render deterministic no-hardware SVG frames into `build/frames/`.
- `npm run compare:frames` - compare generated frame hashes with checked-in expected hashes.
- `npm run emulate` - run Linux, Android, iOS, and Windows no-hardware platform emulators and write transcripts into `build/emulators/`.
- `npm run check:host-emulators` - report native host emulator availability and blockers.
- `npm run verify:platform-scaffolds` - validate platform scaffolds without touching Android runtime state.
- `npm run serve:api` - start the local HTTP API transport.
- `npm run verify` - run fixture validation, tests, frame comparison, frame rendering, platform emulators, host emulator checks, and platform scaffold checks.

- `git status --short --branch` - confirm the active branch and clean working tree.
- `find . -path './.git' -prune -o -path './.agents' -prune -o -path './.codex' -prune -o -type f -print` - list public project files.

## Coding Style & Naming Conventions

Follow the conventions of the language and framework used in this project. Prefer descriptive module names and small, focused files.

## Testing Guidelines

Add tests with new behavior and mirror the source layout where practical. Name tests after the behavior they verify. Every platform branch must keep a no-hardware test path and must preserve the shared UI, API, visualization, input, and capability contract documented in `docs/shared-experience-contract.md`.

## Commit & Pull Request Guidelines

Use clear imperative commit messages. Pull requests should include a short summary, test results, and relevant screenshots for UI changes.

## High-Risk System Changes

Before changing network topology, routing, firewall policy, remote-access exposure, or dangerous permissions on any Windows or Linux device, present the exact proposed changes, reasons, risks, and rollback plan for user approval. Read-only inspection is allowed first; do not implement until approval is explicit.
