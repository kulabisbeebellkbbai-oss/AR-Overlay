# XReal 1S Windows Input Manual Result

Collected: 2026-05-25T13:53:37.4213385-04:00

During the input observation window, press or use any XReal controls that should
produce Windows input.

## Observations

- Any XReal control physically available: mode button, quick mode change, and
  +/- controls are available
- Any Windows keyboard/mouse/media event observed: no windows events observed
- Any visible overlay reaction expected but missing: none
- Shared action mapping candidate: none
- Notes: XReal 1S physical controls are firmware/display controls only. The
  mode button changes view modes, quick mode changes display mode, and +/-
  controls are used for volume, brightness, or firmware menu navigation.

## Result

- Input path usable for AR Overlay shared controls: no
- If not usable, limitation to record: XReal 1S physical controls are
  device-local firmware controls and should not be mapped into the shared AR
  Overlay input contract on Windows. AR Overlay input should use host keyboard,
  mouse, controller, API commands, or a future vendor SDK/external controller
  path.
