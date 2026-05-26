# XReal 1S Windows 11 Bring-Up

Checked into the project on 2026-05-24.

## Hardware State

- First AR/XR hardware target: XReal 1S glasses.
- Host OS: Windows 11.
- Current user-reported state: glasses are connected to the Windows computer and
  working properly.

This moves the Windows branch from no-hardware readiness into hardware bring-up.

## Test Goals

1. Confirm Windows enumerates the glasses as an active display.
2. Confirm the AR Overlay Windows smoke scaffold can run on the Windows host.
3. Confirm a native preview can present the shared no-hardware scene on the
   glasses display.
4. Capture capability, display, transcript, and visual evidence.
5. Record any XReal-specific constraints as Windows adapter facts, not shared
   cross-platform behavior.

## Windows Host Procedure

Run these steps on the Windows 11 computer with the XReal 1S connected.

```powershell
powershell -ExecutionPolicy Bypass -File scripts/windows-xreal-smoke.ps1
```

The script writes results under `build\hardware\xreal-1s-windows11\`.

Because `build/` is ignored by git, copy the generated files into
`hardware-results\xreal-1s-windows11\` before syncing evidence back through the
repo.

Expected evidence:

- `display-info.json`
- `pnp-display-devices.json`
- `dxdiag.txt`
- `ar-overlay-windows-smoke.txt`, if the native smoke binary exists
- `summary.md`

## Evidence Intake Path

Use this tracked folder for synced evidence:

```text
hardware-results\xreal-1s-windows11\
```

Copy from:

```text
build\hardware\xreal-1s-windows11\
```

## Build Step On Windows

If the native smoke binary does not exist yet, configure and build it on the
Windows host:

```powershell
cmake -S platforms/windows -B build/platforms/windows
cmake --build build/platforms/windows --config Debug
```

Then rerun:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/windows-xreal-smoke.ps1
```

## Pass Criteria

- Windows reports at least one active display corresponding to the connected
  glasses path.
- The smoke script completes and writes `summary.md`.
- The Windows native smoke binary runs and emits `platform: windows` capability
  output.
- Any unsupported XReal-specific behavior is documented as a capability or
  adapter limitation.

## First Evidence Result

Evidence synced to the `windows` branch confirms Windows enumerates the XReal
display path:

- DxDiag monitor model: `XREAL 1S`
- Native mode: `1920 x 1200` at `89.999Hz`
- Audio endpoint: `Speakers (2- XREAL 1S)`

The AR Overlay smoke binary was missing during the first run, then successfully
ran during the second synced evidence pass. It emitted Windows capability JSON
and no-hardware lifecycle output.

Detailed analysis is tracked in
`hardware-results/xreal-1s-windows11/analysis.md`.

Next hardware step: build a real Windows preview/presentation path that targets
the XReal display and confirms the shared fixture appears on the glasses.

Run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-preview.ps1
```

This builds `ar-overlay-windows-preview.exe`, opens a borderless preview window
on the XReal/non-primary display for 20 seconds, and writes preview evidence to
`hardware-results\xreal-1s-windows11\`.

The script defaults to Windows display number 2, matching the Windows Settings
screenshot that identifies `Display 2: XREAL 1S`.
On this host, Win32 monitor enumeration exposes that same display as
`\\.\DISPLAY5`, so the preview runner allows fallback to the non-primary
`1920 x 1200` monitor when the Windows Settings number does not map directly.

If Windows changes the display number, rerun with:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-preview.ps1 -DisplayNumber <number>
```

Current synced preview evidence shows successful targeting of `\\.\DISPLAY5`,
the non-primary `1920 x 1200` display corresponding to the XReal path. The
2026-05-25 rerun confirmed the preview was visible on the XReal display, the
text was centered, no image issue was observed, and no Windows display mode
changes were needed.

Recovery note from 2026-05-25: a display topology failure report showed the
XReal PnP monitor present while Win32 monitor enumeration exposed only
`\\.\DISPLAY1`, and `SetDisplayConfig` extend returned `ERROR_GEN_FAILURE`.
The user-reported recovery was a Windows system reboot; after reboot the device
showed up again without issues. If this recurs, capture
`xreal-display-recovery-report.json` before reboot and again after reboot, then
rerun the preview script.

## Remaining Physical-Hardware Work

- Identify whether the first real integration path should use a borderless
  window, Win32 display targeting, DirectX, vendor tooling, or another Windows
  presentation path.

## Timing Test

After preview visibility and placement are confirmed, run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-timing.ps1
```

This rebuilds the Windows preview executable, presents to the XReal target for
30 seconds, records frame-loop timing JSON, and writes synced evidence to
`hardware-results\xreal-1s-windows11\`.

The corrected 2026-05-25 timing run reported `3600` frames over `30000.7 ms`
on the XReal target at `120 Hz`, with average frame interval `8.33134 ms`, no
frames over `20 ms`, and one frame over target plus `2 ms`.

## Input Discovery Test

After presentation timing is accepted, run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-xreal-input-report.ps1
```

This records XReal-related PnP, HID, keyboard, pointing, media, USB, audio, and
network devices before and after a short observation window. During that window,
use any physical XReal controls that should produce Windows input, then complete
`hardware-results\xreal-1s-windows11\input-manual-result.md`.

The 2026-05-25 input discovery report found active XReal HID interfaces for
`VID_3318&PID_043E`, including HID-compliant device, USB Input Device, and
HID-compliant consumer control device entries with `Status: OK`. Manual
observation reported that the physical controls seemed available but no Windows
keyboard, mouse, or media events were observed.

Follow-up hardware clarification: XReal 1S physical controls are device-local
firmware controls. The mode button changes view modes, quick mode changes
display mode, and +/- controls are used for volume, brightness, or firmware
menu navigation. They are not expected to interact with Windows and should not
be mapped into the shared AR Overlay input contract.

AR Overlay input on Windows should come from host keyboard, mouse, controller,
API commands, or a future external controller/vendor SDK path. The checked-in
Raw Input capture tool remains available as an optional diagnostic, but it is
not the next required hardware step for XReal 1S.

## Next Implementation Step

Build the first Windows production presentation backend beyond the GDI preview:
a DirectX/DXGI presentation path that keeps the existing shared scene contract,
targets the validated XReal display path, and preserves the same UI/API/input
behavior as the other platform branches.

Run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-dxgi-preview.ps1
```

This builds `ar-overlay-windows-dxgi-preview.exe`, creates a borderless window
on the validated XReal display, presents through a D3D11/DXGI swap chain, and
writes evidence to `hardware-results\xreal-1s-windows11\`. The initial DXGI
preview validates presentation plumbing with a smooth color field before the
shared scene renderer is moved from the GDI preview into the DirectX path.

The DXGI preview tries hardware D3D11 first and falls back through compatible
swap-chain modes, then WARP, while logging each `create-device-attempt`. If the
first run fails after a display topology issue, reboot the Windows 11 test host,
rerun the script, and sync both stdout and stderr evidence.

The 2026-05-25 DXGI run passed preflight, selected `\\.\DISPLAY5`, created a
hardware D3D11 swap chain with `flip-discard`, and rendered `2237` frames over
`20006.7 ms` with exit code `0`. Manual visual confirmation reports that the
preview was visible on the XReal display, full-screen placement/scaling was
correct, the color field changed smoothly, and no Windows display mode changes
were needed. Minor occasional flicker was observed.

The next DXGI run renders the shared `static-text` scene content through
Direct2D/DirectWrite on the D3D11/DXGI swap chain and emits timing/flicker
instrumentation: target refresh, frame interval min/average/max, frames over
20 ms, frames over 33 ms, frames over target plus 2 ms, and `Present` failures.
The runner forces a clean build of `ar-overlay-windows-dxgi-preview` by default
so Windows does not reuse a stale DXGI binary after source updates. Use
`-SkipCleanBuild` only when intentionally testing an already-built executable.
If the build fails or the binary is not found, the runner copies CMake configure
and build logs into `hardware-results\xreal-1s-windows11\` and, when needed,
writes `cmake-built-executables.txt` with the executable inventory from the
Windows build tree.

The 2026-05-26 shared-scene DXGI run used the clean-built executable and
rendered `scene:"static-text"` with `renderer:"direct2d-directwrite"` on
`\\.\DISPLAY5`. It reported `2234` frames over `20004.6 ms`, average frame
interval `8.95462 ms`, maximum `17.6934 ms`, `0` frames over `20 ms`, `0`
frames over `33 ms`, `168` frames over target plus `2 ms`, and `0` `Present`
failures. Manual visual confirmation reports the scene was visible on XReal,
full-screen placement/scaling was correct, text was visible and centered,
motion/background changes were smooth, no real noticeable flicker was observed,
and no Windows display mode changes were needed. This accepts the
shared-scene DXGI renderer for the current static-text fixture.

Next design path decision: broaden the DXGI renderer to cover the full shared
scene element set, including z-order and image fixtures, or wire the DXGI
presentation path to live API/session updates first.

Current path: live API/session updates first. Start the file-backed API bridge
in one PowerShell window:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-start-dxgi-live-api.ps1
```

Then run the DXGI preview in another PowerShell window:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-dxgi-preview.ps1
```

Send a fixture update through the shared API with:

```powershell
node scripts\send-dxgi-fixture.js --scene=fixtures/scenes/static-text.json
```

The API bridge writes `build\live\windows-dxgi-scene.json`, and the DXGI
preview polls that state file while running. This validates live session/API
updates before broadening the DXGI renderer to every scene element type.
The runner also prints configure/build phase markers and times out the DXGI
target build after 180 seconds by default. Use `-BuildOnly` to isolate build
behavior without running preflight or opening a presentation window, and use
`-BuildTimeoutSeconds <seconds>` if the Windows host is legitimately slower.

The Windows preview, timing, and DXGI scripts run this preflight first:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-xreal-preflight.ps1 -RequireReady
```

If Windows only has phantom XReal PnP entries or the XReal display is missing
from Win32 monitor enumeration, the scripts write
`hardware-results\xreal-1s-windows11\xreal-preflight-report.json` and stop
before opening any presentation window.

When run standalone before a Windows build has produced a preview executable,
preflight accepts an OK present PnP monitor entry for `Generic Monitor (XREAL
1S)` as sufficient readiness evidence. The preview and DXGI scripts still pass
their freshly built executable into preflight so it can also validate the Win32
monitor list before opening a presentation window.
