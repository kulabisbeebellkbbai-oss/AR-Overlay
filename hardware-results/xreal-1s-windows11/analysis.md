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
- Follow-up hardware clarification: the XReal 1S physical controls are
  firmware/display controls only. The mode button changes view modes, quick
  mode changes display mode, and +/- controls are used for volume, brightness,
  or firmware menu navigation. They are not expected to interact with Windows
  or map to AR Overlay shared actions.

## Not Yet Completed

- The Windows Settings screenshot identifies the glasses as `Display 2: XREAL
  1S`, connected to the NVIDIA GeForce RTX 5080 Laptop GPU, with desktop and
  active signal mode `1920 x 1200, 90 Hz`.
- The preview runner later enumerated `\\.\DISPLAY5` as the non-primary
  `1920 x 1200` monitor. That is the Win32 display device corresponding to the
  Windows Settings `Display 2: XREAL 1S` path on this host.
- No photo/screenshot from the glasses view has been synced yet.
- No direct application input path is expected from XReal 1S physical buttons.
  AR Overlay input on Windows should come from host keyboard, mouse,
  controller, API commands, or a future external controller/vendor SDK path.

## Next Required Windows Step

Run the DirectX/DXGI preview path against the validated XReal display target:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows-build-and-xreal-dxgi-preview.ps1
```

The preview, timing, and DXGI scripts now run
`scripts\windows-xreal-preflight.ps1` first. If Windows only has phantom XReal
PnP devices or the XReal monitor is missing from Win32 enumeration, the scripts
write `xreal-preflight-report.json` and refuse to run presentation code.
When preflight is run standalone before a preview executable exists, an OK
present PnP monitor entry for `Generic Monitor (XREAL 1S)` is sufficient to
avoid a false negative.

This is the first Windows production presentation backend beyond the GDI
preview. It should target the same XReal display path and preserve the same
shared scene/UI/API/input behavior. XReal 1S physical buttons are not an AR
Overlay input source, so no additional physical button testing is required
unless a vendor SDK or external controller path is introduced later.

The DXGI runner logs each D3D11 device/swap-chain creation attempt and falls
back across hardware, compatible swap effects, and WARP. If a display topology
failure appears, reboot the Windows 11 test host and rerun the DXGI script
before changing the validated display targeting behavior.

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
- Recurrence captured on 2026-05-25 after a later test run: the XReal 1S again
  disappeared from Win32 desktop monitor enumeration. The diagnose-only
  recovery report at `xreal-display-recovery-report.json` records only
  `\\.\DISPLAY1` in `win32MonitorsBeforeExtend`, with `tryExtendRequested=false`
  so the report did not attempt a topology change. The user planned a cold
  reboot after this commit.
- Restart failure analysis captured on 2026-05-25: Windows produced repeated
  `DRIVER_POWER_STATE_FAILURE (9f)` bugchecks during restart/power transition.
  WinDbg/cdb blackbox PnP data identifies `DISPLAY\MRG4102\5&23fdb3e6&0&UID4356`
  as the affected PnP device path, matching the XReal 1S monitor. Details are
  in `restart-bugcheck-analysis.md`.
- DXGI preview evidence from 2026-05-25 confirms the preflight passed, selected
  `\\.\DISPLAY5`, created a hardware D3D11 swap chain with `flip-discard`, and
  rendered `2237` frames over `20006.7 ms` with exit code `0`.

## Hardware Bring-Up Status

Status: display enumeration confirmed, AR Overlay Windows scaffold runtime
confirmed, preview targeted to the XReal display geometry, reboot recovery
reported successful, manual visibility and placement confirmed on the XReal
display, presentation timing accepted against the reported 120 Hz target.
XReal 1S physical controls are closed as device-local firmware controls, not
AR Overlay application inputs. Windows AR Overlay input should use host input
or API paths. The Windows DirectX/DXGI presentation backend now runs
programmatically on the validated XReal display target; manual visual
confirmation for the DXGI color-field preview remains to be completed before
moving the shared scene renderer from GDI into the DXGI path.
