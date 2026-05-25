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

## Not Yet Completed

- Manual evidence still has TODO values for whether the preview was visible on
  the XReal display and whether placement/scaling was correct.
- No photo/screenshot from the glasses view has been synced yet.
- The Windows Settings screenshot identifies the glasses as `Display 2: XREAL
  1S`, connected to the NVIDIA GeForce RTX 5080 Laptop GPU, with desktop and
  active signal mode `1920 x 1200, 90 Hz`.
- The preview runner later enumerated `\\.\DISPLAY5` as the non-primary
  `1920 x 1200` monitor. That is the Win32 display device corresponding to the
  Windows Settings `Display 2: XREAL 1S` path on this host.
- Real presentation timing has not been measured.
- Display targeting, scaling, and placement behavior still need hands-on
  validation on the Windows host.

## Next Required Windows Step

Rerun the real Windows preview/presentation path on the Windows 11 machine with
the XReal 1S as the target display after the reboot restored display
enumeration.

Implemented next artifact:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-preview.ps1
```

This builds and runs `ar-overlay-windows-preview.exe`.
The preview runner now targets Windows display number 2 by default, and if that
Windows Settings number does not map directly to a Win32 `\\.\DISPLAY<n>` device,
it allows fallback to the non-primary `1920 x 1200` monitor. On this host the
fallback target is `\\.\DISPLAY5`.

Expected result:

- XReal display is selected as the presentation target.
- The shared scene is visible on the XReal display.
- Scaling, refresh mode, and placement observations are captured.
- If the XReal display disappears from Win32 enumeration again, capture
  `xreal-display-recovery-report.json` before and after reboot. The current
  working recovery action is a Windows system reboot.

## Hardware Bring-Up Status

Status: display enumeration confirmed, AR Overlay Windows scaffold runtime
confirmed, preview targeted to the XReal display geometry, reboot recovery
reported successful, manual visibility confirmation pending after the reboot.
