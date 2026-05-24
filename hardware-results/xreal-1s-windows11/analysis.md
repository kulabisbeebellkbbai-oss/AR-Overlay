# XReal 1S Windows 11 Evidence Analysis

Analyzed from the `windows` branch evidence synced on 2026-05-24.

## Confirmed

- Windows reports two active monitor paths.
- DxDiag identifies the external monitor model as `XREAL 1S`.
- The XReal display native mode is reported as `1920 x 1200` at `89.999Hz`.
- XReal audio is present as `Speakers (2- XREAL 1S)`.
- XReal capture/input terminal entries are present.

## Not Completed

- `ar-overlay-windows-smoke.txt` reports that the AR Overlay native smoke binary
  was not found.
- That means the Windows host evidence confirms the glasses/display path, but
  does not yet confirm the AR Overlay Windows scaffold running on the Windows
  machine.

## Next Required Windows Step

Run the Windows build-and-smoke helper from the repo root on the Windows 11
machine:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-smoke.ps1
```

Expected result:

- CMake configures `platforms/windows`.
- The Windows smoke binary is built.
- `scripts/windows-xreal-smoke.ps1` reruns and captures smoke output.
- `hardware-results\xreal-1s-windows11\ar-overlay-windows-smoke.txt` contains
  Windows capability JSON instead of the missing-binary message.

## Hardware Bring-Up Status

Status: display enumeration confirmed, AR Overlay Windows scaffold runtime
pending.
