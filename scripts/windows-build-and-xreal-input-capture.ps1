param(
    [int]$DurationSeconds = 20,
    [string]$OutDir = "build\hardware\xreal-1s-windows11",
    [string]$SyncDir = "hardware-results\xreal-1s-windows11"
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $SyncDir | Out-Null

cmake -S platforms/windows -B build/platforms/windows
cmake --build build/platforms/windows --config Debug

$capture = "build\platforms\windows\Debug\ar-overlay-windows-raw-input-capture.exe"
if (-not (Test-Path $capture)) {
    $capture = "build\platforms\windows\ar-overlay-windows-raw-input-capture.exe"
}
if (-not (Test-Path $capture)) {
    throw "raw input capture binary not found"
}

$outputPath = Join-Path $OutDir "ar-overlay-windows-raw-input-capture.txt"
$errorPath = Join-Path $OutDir "ar-overlay-windows-raw-input-capture.err.txt"

Write-Host "Starting XReal raw input capture for $DurationSeconds seconds."
Write-Host "Use any physical XReal controls during the capture window."

& $capture --duration=$DurationSeconds 1> $outputPath 2> $errorPath
$exitCode = $LASTEXITCODE

Copy-Item -Force $outputPath (Join-Path $SyncDir "ar-overlay-windows-raw-input-capture.txt")
Copy-Item -Force $errorPath (Join-Path $SyncDir "ar-overlay-windows-raw-input-capture.err.txt")

$manualPath = Join-Path $OutDir "raw-input-manual-result.md"
$timestamp = Get-Date -Format o
$manual = @"
# XReal 1S Windows Raw Input Manual Result

Collected: $timestamp

During the raw input capture window, press or use any physical XReal controls.

## Observations

- Any visible Windows event from the glasses controls: TODO
- Any AR Overlay raw input event with `"xreal":true`: TODO
- Any HID bytes changing when controls are used: TODO
- Candidate shared action mapping: TODO
- Notes: TODO

## Result

- Raw Input path usable for AR Overlay shared controls: TODO
- If not usable, limitation to record: TODO
"@
[System.IO.File]::WriteAllText(
    (Resolve-Path -LiteralPath (Split-Path -Parent $manualPath)).Path + [System.IO.Path]::DirectorySeparatorChar + (Split-Path -Leaf $manualPath),
    $manual,
    [System.Text.UTF8Encoding]::new($false))
Copy-Item -Force $manualPath (Join-Path $SyncDir "raw-input-manual-result.md")

if ($exitCode -ne 0) {
    throw "raw input capture failed with exit code $exitCode"
}

Write-Host "Copied raw input evidence to $SyncDir"
Write-Host "Complete hardware-results\xreal-1s-windows11\raw-input-manual-result.md before syncing results."
