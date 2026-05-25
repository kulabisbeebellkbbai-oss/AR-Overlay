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

The smoke script writes these files under
`build\hardware\xreal-1s-windows11\` on the Windows machine. Because `build/`
is ignored by git, copy the files into this tracked intake folder before
syncing through git.
