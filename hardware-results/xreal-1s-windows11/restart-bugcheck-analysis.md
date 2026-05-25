# XReal 1S Restart Bugcheck Analysis

Recorded on 2026-05-25 after the XReal 1S display disappeared from Win32
desktop monitor enumeration and the user performed a restart.

## Tooling Installed

- Installed WinDbg through `winget install --id Microsoft.WinDbg`.
- Installed Windows SDK desktop debugging tools with:
  `winsdksetup.exe /quiet /norestart /features OptionId.WindowsDesktopDebuggers`.
- Used `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\cdb.exe`.

## Dumps Analyzed

- `C:\Windows\Minidump\052526-16437-01.dmp`
  - Copied locally to `build\hardware\xreal-1s-windows11\052526-16437-01.dmp`
  - Crash time: 2026-05-25 15:55 local
  - Uptime: 15:54:17
- `C:\Windows\Minidump\052526-16796-01.dmp`
  - Copied locally to `build\hardware\xreal-1s-windows11\052526-16796-01.dmp`
  - Crash time: 2026-05-25 00:00 local
  - Uptime: 6 days 13:00:22

The minidump binaries are not tracked in git. Raw debugger transcripts remain
under ignored `build\hardware\xreal-1s-windows11\`.

## Shared Bugcheck Signature

Both dumps report the same Windows bugcheck:

```text
DRIVER_POWER_STATE_FAILURE (9f)
Arg1: 0000000000000004
Arg2: 000000000000012c
Failure bucket: 0x9F_4_nt!PnpBugcheckPowerTimeout
```

Interpretation: Windows timed out for 300 seconds while synchronizing with the
PnP subsystem during a power transition. This is a driver/device
power-transition failure, not an AR Overlay application exception.

## PnP Blackbox Evidence

Targeted debugger commands:

```text
!thread <arg3> 1f
!blackboxpnp
dt nt!_TRIAGE_9F_PNP <arg4>
```

Both dumps identify the same PnP device:

```text
PnpEventInformation: 3
PnpEventInProgress : 0
PnpProblemCode     : 24
DeviceId           : DISPLAY\MRG4102\5&23fdb3e6&0&UID4356
```

`DISPLAY\MRG4102...` is the XReal 1S monitor path observed throughout the
Windows bring-up evidence.

## Current Post-Crash Device State

After the 15:55 crash/restart:

- `scripts\windows-xreal-recover-display.ps1` reports only `\\.\DISPLAY1` in
  Win32 monitor enumeration.
- `Get-PnpDevice -PresentOnly` does not show XReal / `VID_3318` devices.
- Non-present XReal entries are all `CM_PROB_PHANTOM`, including the monitor,
  HID, audio, USB composite, NCM, and CDC ECM interfaces.

The latest post-crash recovery report is tracked as:

```text
hardware-results\xreal-1s-windows11\xreal-display-recovery-report.json
```

## Working Assessment

The repeated crash is tied to Windows PnP/power handling of the XReal display
device path, with USB/audio/network companion interfaces present in the device
history. AR Overlay can trigger or expose the state by exercising preview and
display enumeration, but the actual crash is in Windows driver power-transition
handling for the XReal monitor path.

Do not repeatedly run preview or topology-changing recovery commands while this
signature is active. Use diagnose-only reporting first.

## Next Engineering Steps

- Keep `scripts\windows-xreal-recover-display.ps1` diagnose-only by default.
- Add richer inactive display-path capture with `QueryDisplayConfig`, including
  source IDs, target IDs, output technology, status flags, and friendly names.
- Add a hardware-safe preflight before any Windows preview run:
  - confirm XReal is present in Win32 monitor enumeration,
  - confirm XReal is not only a phantom PnP device,
  - refuse to run topology-changing actions unless explicitly requested.
- If recurrence continues, analyze full kernel dumps or newer WinDbg output to
  isolate the lower driver stack beyond the PnP blackbox device ID.
