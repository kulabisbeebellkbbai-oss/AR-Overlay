# Linux Hardware Branch

## Branch Goal

Build the first locally verifiable AR Overlay host while keeping Linux support
as distribution-agnostic as practical.

## Baseline

Primary development should avoid hard dependencies on a single distribution.
When a concrete host path is required, use this workstation:

- Debian GNU/Linux 13.5 (`trixie`)
- Kernel `6.12.86+deb13-amd64`
- `x86_64`

## Adapter Direction

- Start with a no-hardware display simulator before binding to a glasses SDK.
- Keep compositor handling behind an adapter that can support Wayland first and
  X11 where practical.
- Treat device discovery, permissions, and graphics backend selection as
  runtime capabilities.
- Prefer open protocols and common graphics paths before vendor SDK bindings.

## Initial Work Items

1. Create a Linux capability probe.
2. Create a no-hardware renderer that consumes shared visual fixtures.
3. Add display lifecycle commands: create session, present frame, update scene,
   and close session.
4. Add input action replay from shared fixtures.
5. Add optional hardware notes once the first glasses or display bridge is
   selected.

## Compatibility Notes

- Do not encode Debian-only assumptions into shared code.
- Debian-specific package or service commands belong in Linux docs or scripts
  only.
- If a vendor SDK requires a specific compositor, driver, or kernel module,
  document that as a capability requirement.
