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

- Validate physical input path, if any.
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

The report does not decode HID event payloads. If Windows exposes only a HID
device with no observable keyboard, mouse, or media event, the next software
step is a native Raw Input/HID listener. If Windows exposes no usable input
path, record that as an XReal 1S Windows capability limitation.

The 2026-05-25 input discovery report found active XReal HID interfaces for
`VID_3318&PID_043E`, including HID-compliant device, USB Input Device, and
HID-compliant consumer control device entries with `Status: OK`. Manual
observation reported that the physical controls seemed available but no Windows
keyboard, mouse, or media events were observed.

## Raw Input Capture Test

Run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-input-capture.ps1
```

This builds and runs `ar-overlay-windows-raw-input-capture.exe`, enumerates Raw
Input devices, listens for keyboard, mouse, consumer-control, system-control,
and vendor-defined HID packets, and writes synced evidence to
`hardware-results\xreal-1s-windows11\`. During the capture window, use any
physical XReal controls. Then complete
`hardware-results\xreal-1s-windows11\raw-input-manual-result.md`.

Pass criteria: the capture output includes one or more `raw-input` events with
`"xreal":true` while a physical control is used. If that occurs, map the HID
payloads into the shared AR Overlay input contract. If no XReal events are
captured, generic Windows input for XReal 1S should be recorded as unsupported
pending vendor SDK or deeper USB/HID integration.
