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
- `xreal-preflight-report.json`, before running preview, timing, or DXGI
  presentation scripts
- `ar-overlay-windows-timing.txt` and `timing-manual-result.md`, when
  collecting presentation timing evidence
- `ar-overlay-windows-dxgi-preview.txt`,
  `ar-overlay-windows-dxgi-preview.err.txt`, and
  `dxgi-preview-manual-result.md`, when collecting DirectX/DXGI presentation
  evidence
- `cmake-configure-dxgi-preview.txt`,
  `cmake-configure-dxgi-preview.err.txt`,
  `cmake-build-dxgi-preview.txt`, and `cmake-build-dxgi-preview.err.txt`, when
  collecting DirectX/DXGI build evidence or diagnosing build failures
- `build\live\windows-dxgi-scene.json`, ignored by git, is the live state file
  used by the Windows DXGI API bridge during live-update tests
- `xreal-input-report.json` and `input-manual-result.md`, when collecting
  physical input discovery evidence
- `ar-overlay-windows-raw-input-capture.txt`,
  `ar-overlay-windows-raw-input-capture.err.txt`, and
  `raw-input-manual-result.md`, when collecting native Raw Input/HID evidence

The smoke script writes these files under
`build\hardware\xreal-1s-windows11\` on the Windows machine. Because `build/`
is ignored by git, copy the files into this tracked intake folder before
syncing through git.
