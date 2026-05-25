param(
    [int]$DurationSeconds = 30,
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

$timingOut = Join-Path $OutDir "ar-overlay-windows-timing.txt"
$timingErr = Join-Path $OutDir "ar-overlay-windows-timing.err.txt"
if (Test-Path $timingOut) { Remove-Item -Force $timingOut }
if (Test-Path $timingErr) { Remove-Item -Force $timingErr }

$previewArgs = @(
    "--target=$Target",
    "--display-number=$DisplayNumber",
    "--duration=$DurationSeconds",
    "--require-target",
    "--allow-fallback",
    "--measure-timing"
)

$process = Start-Process -FilePath $preview -ArgumentList $previewArgs -Wait -PassThru -NoNewWindow -RedirectStandardOutput $timingOut -RedirectStandardError $timingErr
$timingExit = $process.ExitCode
Get-Content $timingOut
if ($timingExit -ne 0) {
    Get-Content $timingErr
}

$timingJson = Get-Content $timingOut |
    Where-Object { $_ -match '"mode":"xreal-preview-timing"' } |
    Select-Object -Last 1

$timing = $null
if ($timingJson) {
    $timing = $timingJson | ConvertFrom-Json
}

$summary = @"
# XReal 1S Timing Manual Result

- Date: $(Get-Date -Format o)
- Target selector: $Target
- Windows display number: $DisplayNumber
- Duration seconds: $DurationSeconds
- Timing output: $timingOut
- Timing error output: $timingErr
- Timing exit code: $timingExit

Measured timing:

- Target refresh Hz: $(if ($timing) { $timing.targetRefreshHz } else { "unknown" })
- Frame count: $(if ($timing) { $timing.frameCount } else { "unknown" })
- Average frame interval ms: $(if ($timing) { $timing.avgFrameMs } else { "unknown" })
- Minimum frame interval ms: $(if ($timing) { $timing.minFrameMs } else { "unknown" })
- Maximum frame interval ms: $(if ($timing) { $timing.maxFrameMs } else { "unknown" })
- Frames over 20 ms: $(if ($timing) { $timing.framesOver20Ms } else { "unknown" })
- Frames over 33 ms: $(if ($timing) { $timing.framesOver33Ms } else { "unknown" })

Manual observations to fill in:

- Timing run visible on XReal display: TODO
- Any visible stutter or flicker: TODO
- Comfort notes during timing run: TODO
"@

$summaryPath = Join-Path $OutDir "timing-manual-result.md"
$summary | Set-Content -Path $summaryPath -Encoding UTF8

Copy-Item -Force -Path $timingOut, $timingErr, $summaryPath -Destination $SyncDir
Write-Host "Copied timing evidence to $SyncDir"

if ($timingExit -ne 0) {
    throw "Timing preview did not find the requested target display. Confirm Windows is in Extend mode and rerun."
}
