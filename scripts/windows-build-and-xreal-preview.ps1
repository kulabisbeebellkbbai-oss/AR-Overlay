param(
    [int]$DurationSeconds = 20,
    [string]$Target = "XREAL",
    [int]$DisplayNumber = 2,
    [string]$OutDir = "build\hardware\xreal-1s-windows11",
    [string]$SyncDir = "hardware-results\xreal-1s-windows11"
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake is required on the Windows host. Install CMake or add it to PATH, then rerun this script."
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $SyncDir | Out-Null

cmake -S platforms/windows -B build/platforms/windows
cmake --build build/platforms/windows --config Debug

$previewCandidates = @(
    "build\platforms\windows\Debug\ar-overlay-windows-preview.exe",
    "build\platforms\windows\ar-overlay-windows-preview.exe"
)

$preview = $previewCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $preview) {
    throw "Preview binary not found after build."
}

$previewOut = Join-Path $OutDir "ar-overlay-windows-preview.txt"
$previewErr = Join-Path $OutDir "ar-overlay-windows-preview.err.txt"
if (Test-Path $previewOut) { Remove-Item -Force $previewOut }
if (Test-Path $previewErr) { Remove-Item -Force $previewErr }

$previewArgs = @(
    "--target=$Target",
    "--display-number=$DisplayNumber",
    "--duration=$DurationSeconds",
    "--require-target",
    "--allow-fallback"
)

$process = Start-Process -FilePath $preview -ArgumentList $previewArgs -Wait -PassThru -NoNewWindow -RedirectStandardOutput $previewOut -RedirectStandardError $previewErr
$previewExit = $process.ExitCode
Get-Content $previewOut
if ($previewExit -ne 0) {
    Get-Content $previewErr
}

$manual = @"
# XReal 1S Preview Manual Result

- Date: $(Get-Date -Format o)
- Target selector: $Target
- Windows display number: $DisplayNumber
- Duration seconds: $DurationSeconds
- Preview output: $previewOut
- Preview error output: $previewErr
- Preview exit code: $previewExit

Manual observations to fill in:

- Preview visible on XReal display: TODO
- Placement/scaling correct: TODO
- Flicker/latency/comfort notes: TODO
- Any Windows display mode changes needed: TODO
"@

$manualPath = Join-Path $OutDir "preview-manual-result.md"
$manual | Set-Content -Path $manualPath -Encoding UTF8

Copy-Item -Force -Path $previewOut, $previewErr, $manualPath -Destination $SyncDir
Get-ChildItem -Path $SyncDir -Filter "Screenshot*.png" -ErrorAction SilentlyContinue | Out-Null
Write-Host "Copied preview evidence to $SyncDir"

if ($previewExit -ne 0) {
    throw "Preview did not find the requested target display. Confirm Windows is in Extend mode and rerun."
}
