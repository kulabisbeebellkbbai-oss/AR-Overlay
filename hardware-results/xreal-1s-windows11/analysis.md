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

## Not Yet Completed

- The evidence does not yet prove an overlay window or rendered preview appeared
  on the XReal display.
- Real presentation timing has not been measured.
- Display targeting, scaling, and placement behavior still need hands-on
  validation on the Windows host.

## Next Required Windows Step

Build the first real Windows preview/presentation path and run it on the Windows
11 machine with the XReal 1S as the target display.

The next useful artifact should create a borderless preview window or equivalent
targeted presentation surface, render the shared fixture, and capture manual
confirmation of placement on the glasses.

Implemented next artifact:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-preview.ps1
```

This builds and runs `ar-overlay-windows-preview.exe`.

Expected result:

- XReal display is selected as the presentation target.
- The shared scene is visible on the XReal display.
- Scaling, refresh mode, and placement observations are captured.

## Hardware Bring-Up Status

Status: display enumeration confirmed, AR Overlay Windows scaffold runtime
confirmed, real overlay presentation pending.
