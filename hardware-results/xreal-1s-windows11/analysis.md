# XReal 1S Windows 11 Evidence Analysis

Analyzed from the `windows` branch evidence synced on 2026-05-24.

## Confirmed

- Windows reports two active monitor paths.
- DxDiag identifies the external monitor model as `XREAL 1S`.
- The XReal display native mode is reported as `1920 x 1200` at `89.999Hz`.
- XReal audio is present as `Speakers (XREAL 1S)`.
- XReal capture/input terminal entries are present.
- The AR Overlay Windows smoke binary runs on the Windows host.
- The smoke binary emits Windows capability JSON.
- The smoke binary exercises the no-hardware lifecycle:
  `createDisplaySession`, `presentScene:windows-no-hardware`,
  `translateInput:select`, `translateInput:dismiss`, and `shutdown`.
- The Windows preview runner now resolves the target to `\\.\DISPLAY5`, the
  non-primary `1920 x 1200` monitor at desktop position `x=2560, y=0`.
- Preview run exit code is `0`, with no stderr output.
- A later recovery test showed the XReal PnP monitor present but Win32 monitor
  enumeration temporarily collapsed to only `\\.\DISPLAY1`; `SetDisplayConfig`
  extend returned `ERROR_GEN_FAILURE`.
- User-reported follow-up on 2026-05-25: after a Windows system reboot, the
  XReal display appeared again without issues.
- Re-run preview evidence from 2026-05-25 confirms the shared preview was
  visible on the XReal display, with text centered, no visible image issue, and
  no Windows display mode changes required.
- Corrected timing evidence from 2026-05-25 confirms the preview loop matched
  the reported `120 Hz` target: `3600` frames over `30000.7 ms`, average frame
  interval `8.33134 ms`, maximum `12.8139 ms`, `0` frames over `20 ms`, and `1`
  frame over target plus `2 ms`.
- Input discovery from 2026-05-25 shows active XReal HID interfaces for
  `VID_3318&PID_043E`, including HID-compliant device, USB Input Device, and
  HID-compliant consumer control device entries with `Status: OK`.
- During the input discovery observation window, the tester reported that all
  physical controls seemed available but no Windows keyboard, mouse, or media
  events were observed.

## Not Yet Completed

- The Windows Settings screenshot identifies the glasses as `Display 2: XREAL
  1S`, connected to the NVIDIA GeForce RTX 5080 Laptop GPU, with desktop and
  active signal mode `1920 x 1200, 90 Hz`.
- The preview runner later enumerated `\\.\DISPLAY5` as the non-primary
  `1920 x 1200` monitor. That is the Win32 display device corresponding to the
  Windows Settings `Display 2: XREAL 1S` path on this host.
- No photo/screenshot from the glasses view has been synced yet.
- Physical input behavior, if exposed by the XReal path, has not been validated.

## Next Required Windows Step

Run the native Raw Input capture and validate whether the XReal HID interfaces
produce packets that can be mapped to shared AR Overlay actions:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-input-capture.ps1
```

If the capture records XReal HID packets, implement a Windows input adapter
mapping those packets to the shared action contract. If no XReal Raw Input
events are emitted while controls are used, record input as unsupported through
the generic Windows input path and continue toward the first DirectX/vendor-SDK
integration path.

Implemented next artifact:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-preview.ps1
```

This builds and runs `ar-overlay-windows-preview.exe`.
The preview runner now targets Windows display number 2 by default, and if that
Windows Settings number does not map directly to a Win32 `\\.\DISPLAY<n>` device,
it allows fallback to the non-primary `1920 x 1200` monitor. On this host the
fallback target is `\\.\DISPLAY5`.

Current preview result:

- XReal display is selected as the presentation target.
- The shared scene is visible on the XReal display.
- Text placement is centered.
- No image visibility issue was reported.
- No Windows display mode changes were needed.
- Presentation timing meets the reported `120 Hz` display target in the
  corrected timing run.
- If the XReal display disappears from Win32 enumeration again, capture
  `xreal-display-recovery-report.json` before and after reboot. The current
  working recovery action is a Windows system reboot.

## Hardware Bring-Up Status

Status: display enumeration confirmed, AR Overlay Windows scaffold runtime
confirmed, preview targeted to the XReal display geometry, reboot recovery
reported successful, manual visibility and placement confirmed on the XReal
display, presentation timing accepted against the reported 120 Hz target.
Physical input has visible HID device exposure but no generic Windows input
events yet. Native Raw Input/HID event capture remains to be validated or the
path must be explicitly marked unsupported.
