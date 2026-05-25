# XReal 1S Windows 11 Evidence Intake

Place synced Windows smoke results for the XReal 1S bring-up in this folder.

Expected files from the Windows host:

- `summary.md`
- `display-info.json`
- `pnp-display-devices.json`
- `dxdiag.txt`
- `ar-overlay-windows-smoke.txt`
- `xreal-display-recovery-report.json`, when collecting a display-topology
  failure report
- `ar-overlay-windows-timing.txt` and `timing-manual-result.md`, when
  collecting presentation timing evidence
- `xreal-input-report.json` and `input-manual-result.md`, when collecting
  physical input discovery evidence
- `ar-overlay-windows-raw-input-capture.txt`,
  `ar-overlay-windows-raw-input-capture.err.txt`, and
  `raw-input-manual-result.md`, when collecting native Raw Input/HID evidence

The smoke script writes these files under
`build\hardware\xreal-1s-windows11\` on the Windows machine. Because `build/`
is ignored by git, copy the files into this tracked intake folder before
syncing through git.
