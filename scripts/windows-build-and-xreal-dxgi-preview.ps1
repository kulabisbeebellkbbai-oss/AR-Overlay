param(
    [int]$DurationSeconds = 20,
    [string]$Target = "XREAL",
    [int]$DisplayNumber = 2,
    [string]$OutDir = "build\hardware\xreal-1s-windows11",
    [string]$SyncDir = "hardware-results\xreal-1s-windows11",
    [switch]$SkipCleanBuild,
    [int]$BuildTimeoutSeconds = 180,
    [switch]$BuildOnly
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake is required on the Windows host. Install CMake or add it to PATH, then rerun this script."
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $SyncDir | Out-Null

$configureOut = Join-Path $OutDir "cmake-configure-dxgi-preview.txt"
$configureErr = Join-Path $OutDir "cmake-configure-dxgi-preview.err.txt"
$buildOut = Join-Path $OutDir "cmake-build-dxgi-preview.txt"
$buildErr = Join-Path $OutDir "cmake-build-dxgi-preview.err.txt"

if (Test-Path $configureOut) { Remove-Item -Force $configureOut }
if (Test-Path $configureErr) { Remove-Item -Force $configureErr }
if (Test-Path $buildOut) { Remove-Item -Force $buildOut }
if (Test-Path $buildErr) { Remove-Item -Force $buildErr }

Write-Host "Configuring Windows DXGI preview..."
$configureProcess = Start-Process -FilePath "cmake" `
    -ArgumentList @("-S", "platforms/windows", "-B", "build/platforms/windows") `
    -Wait `
    -PassThru `
    -NoNewWindow `
    -RedirectStandardOutput $configureOut `
    -RedirectStandardError $configureErr
Get-Content $configureOut
if ($configureProcess.ExitCode -ne 0) {
    Get-Content $configureErr
    Copy-Item -Force -Path $configureOut, $configureErr -Destination $SyncDir
    throw "CMake configure failed with exit code $($configureProcess.ExitCode). See $configureOut and $configureErr."
}
Write-Host "Configure completed."

if ($SkipCleanBuild) {
    $buildArgs = @("--build", "build/platforms/windows", "--config", "Debug", "--target", "ar-overlay-windows-dxgi-preview")
} else {
    $buildArgs = @("--build", "build/platforms/windows", "--config", "Debug", "--target", "ar-overlay-windows-dxgi-preview", "--clean-first")
}

Write-Host "Building Windows DXGI preview target..."
$buildProcess = Start-Process -FilePath "cmake" `
    -ArgumentList $buildArgs `
    -PassThru `
    -NoNewWindow `
    -RedirectStandardOutput $buildOut `
    -RedirectStandardError $buildErr

if (-not $buildProcess.WaitForExit($BuildTimeoutSeconds * 1000)) {
    Write-Host "DXGI preview build timed out after $BuildTimeoutSeconds seconds."
    try {
        Stop-Process -Id $buildProcess.Id -Force -ErrorAction SilentlyContinue
    } catch {
        Write-Host "Could not stop build process $($buildProcess.Id): $_"
    }
    Copy-Item -Force -Path $configureOut, $configureErr, $buildOut, $buildErr -Destination $SyncDir
    if (Test-Path $buildOut) { Get-Content $buildOut }
    if (Test-Path $buildErr) { Get-Content $buildErr }
    throw "DXGI preview build timed out after $BuildTimeoutSeconds seconds. See $buildOut and $buildErr."
}

Get-Content $buildOut
if ($buildProcess.ExitCode -ne 0) {
    Get-Content $buildErr
    Copy-Item -Force -Path $configureOut, $configureErr, $buildOut, $buildErr -Destination $SyncDir
    throw "DXGI preview build failed with exit code $($buildProcess.ExitCode). See $buildOut and $buildErr."
}
Write-Host "Build completed."

$previewCandidates = @(
    "build\platforms\windows\Debug\ar-overlay-windows-dxgi-preview.exe",
    "build\platforms\windows\ar-overlay-windows-dxgi-preview.exe"
)

$preview = $previewCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $preview) {
    $preview = Get-ChildItem -Path "build\platforms\windows" -Recurse -Filter "ar-overlay-windows-dxgi-preview.exe" -ErrorAction SilentlyContinue |
        Select-Object -ExpandProperty FullName -First 1
}
if (-not $preview) {
    $exeInventory = Join-Path $OutDir "cmake-built-executables.txt"
    Get-ChildItem -Path "build\platforms\windows" -Recurse -Filter "*.exe" -ErrorAction SilentlyContinue |
        Select-Object -ExpandProperty FullName |
        Set-Content -Path $exeInventory -Encoding UTF8
    Copy-Item -Force -Path $configureOut, $configureErr, $buildOut, $buildErr, $exeInventory -Destination $SyncDir
    throw "DXGI preview binary not found after successful build. See $exeInventory for executable inventory."
}
Write-Host "Using DXGI preview binary: $preview"
if ($BuildOnly) {
    Copy-Item -Force -Path $configureOut, $configureErr, $buildOut, $buildErr -Destination $SyncDir
    Write-Host "BuildOnly requested; copied build evidence to $SyncDir"
    exit 0
}

& powershell -ExecutionPolicy Bypass -File scripts\windows-xreal-preflight.ps1 `
    -Target $Target `
    -DisplayNumber $DisplayNumber `
    -MonitorListExecutable $preview `
    -OutDir $OutDir `
    -SyncDir $SyncDir `
    -RequireReady

$previewOut = Join-Path $OutDir "ar-overlay-windows-dxgi-preview.txt"
$previewErr = Join-Path $OutDir "ar-overlay-windows-dxgi-preview.err.txt"
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
# XReal 1S DXGI Preview Manual Result

- Date: $(Get-Date -Format o)
- Target selector: $Target
- Windows display number: $DisplayNumber
- Duration seconds: $DurationSeconds
- Clean build used: $(-not [bool]$SkipCleanBuild)
- DXGI preview output: $previewOut
- DXGI preview error output: $previewErr
- DXGI preview exit code: $previewExit

Manual observations to fill in:

- DXGI preview visible on XReal display: TODO
- Full-screen placement/scaling correct: TODO
- Shared scene text visible and centered: TODO
- Motion/background changes smoothly: TODO
- Flicker/latency/comfort notes: TODO
- Any Windows display mode changes needed: TODO
"@

$manualPath = Join-Path $OutDir "dxgi-preview-manual-result.md"
$manual | Set-Content -Path $manualPath -Encoding UTF8

Copy-Item -Force -Path $previewOut, $previewErr, $manualPath -Destination $SyncDir
Copy-Item -Force -Path $configureOut, $configureErr, $buildOut, $buildErr -Destination $SyncDir
Write-Host "Copied DXGI preview evidence to $SyncDir"

if ($previewExit -ne 0) {
    throw "DXGI preview did not find or present to the requested target display. Confirm Windows is in Extend mode and rerun."
}
