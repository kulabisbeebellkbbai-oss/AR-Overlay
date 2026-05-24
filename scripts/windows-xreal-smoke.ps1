param(
    [string]$OutDir = "build\hardware\xreal-1s-windows11"
)

$ErrorActionPreference = "Stop"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$displayInfoPath = Join-Path $OutDir "display-info.json"
$pnpPath = Join-Path $OutDir "pnp-display-devices.json"
$dxdiagPath = Join-Path $OutDir "dxdiag.txt"
$smokePath = Join-Path $OutDir "ar-overlay-windows-smoke.txt"
$summaryPath = Join-Path $OutDir "summary.md"

Get-CimInstance -Namespace root\wmi -ClassName WmiMonitorBasicDisplayParams |
    Select-Object InstanceName, Active, MaxHorizontalImageSize, MaxVerticalImageSize |
    ConvertTo-Json -Depth 4 |
    Set-Content -Path $displayInfoPath -Encoding UTF8

Get-PnpDevice -Class Display |
    Select-Object Status, Class, FriendlyName, InstanceId |
    ConvertTo-Json -Depth 4 |
    Set-Content -Path $pnpPath -Encoding UTF8

dxdiag /t $dxdiagPath | Out-Null

$candidateBinaries = @(
    "build\platforms\windows\Debug\ar-overlay-windows-smoke.exe",
    "build\platforms\windows\ar-overlay-windows-smoke.exe",
    "build\platforms\windows-host-check\ar-overlay-windows-smoke.exe"
)

$smokeBinary = $candidateBinaries | Where-Object { Test-Path $_ } | Select-Object -First 1
if ($smokeBinary) {
    & $smokeBinary | Set-Content -Path $smokePath -Encoding UTF8
} else {
    "Smoke binary not found. Build with: cmake -S platforms/windows -B build/platforms/windows; cmake --build build/platforms/windows --config Debug" |
        Set-Content -Path $smokePath -Encoding UTF8
}

$summary = @"
# XReal 1S Windows 11 Smoke Summary

- Date: $(Get-Date -Format o)
- Output directory: $OutDir
- Display info: $displayInfoPath
- PnP display devices: $pnpPath
- DxDiag report: $dxdiagPath
- AR Overlay smoke output: $smokePath

Manual confirmation required:

- XReal 1S visible as an active Windows display.
- Overlay preview appears on the glasses display.
- Any display mode, refresh rate, scaling, or placement issue is recorded.
"@

$summary | Set-Content -Path $summaryPath -Encoding UTF8
Write-Host "Wrote $summaryPath"
