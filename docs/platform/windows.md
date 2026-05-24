# Windows Hardware Branch

## Branch Goal

Build the Windows AR Overlay adapter for current Windows 11 while preserving the
shared UI, API, visualization, and input contract.

## Baseline

- Primary OS target: current Windows 11 general availability state.
- Compatibility assumption: support Windows 11 25H2 and 24H2 unless hardware or
  vendor SDK requirements force a narrower target.
- Windows 11 26H1 is treated as new-device support unless a selected hardware
  target requires it.

## Adapter Direction

- Keep WinUI, graphics, display capture, sensor, driver, and vendor SDK choices
  behind adapter interfaces.
- Prefer capability probes over build-number checks.
- Keep hardware bridge code optional until the first Windows-connected glasses
  target is selected.
- Preserve shared visual fixtures and API messages regardless of native UI stack.

## Initial Work Items

1. Create a Windows capability probe for OS build, graphics backend, display,
   input, permissions, and attached hardware bridge state.
2. Create a no-hardware renderer that consumes shared visual fixtures.
3. Add shared API message validation before Windows command execution.
4. Add input action translation from keyboard, pointer, controller, or glasses
   bridge events to shared actions.
5. Add packaging and local run documentation after the implementation stack is
   selected.

## Compatibility Notes

- Do not require Windows 11 26H1 unless selected hardware makes it necessary.
- Keep SDK-specific behavior behind adapter diagnostics and capability reports.
- Do not expose Windows-specific lifecycle, HRESULT, or driver vocabulary in
  shared API responses; map it to shared session and error states.
