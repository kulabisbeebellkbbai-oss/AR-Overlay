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

1. Use `createPlatformAdapter("windows")` for the current no-hardware
   capability probe.
2. Use `npm run render:fixtures` for the current no-hardware renderer.
3. Use the shared command processor for API message validation before Windows
   command execution.
4. Use shared input fixtures as the target shape for keyboard, pointer,
   controller, or glasses bridge event translation.
5. Add packaging and local run documentation after the native Windows adapter
   stack is selected.

## No-Hardware Verification

Run `npm run verify` on this branch before hardware or Windows SDK work begins.

## Compatibility Notes

- Do not require Windows 11 26H1 unless selected hardware makes it necessary.
- Keep SDK-specific behavior behind adapter diagnostics and capability reports.
- Do not expose Windows-specific lifecycle, HRESULT, or driver vocabulary in
  shared API responses; map it to shared session and error states.
