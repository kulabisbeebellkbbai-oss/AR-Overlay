param(
    [string]$Target = "XREAL",
    [int]$DisplayNumber = 2,
    [string]$MonitorListExecutable = "",
    [string]$OutDir = "build\hardware\xreal-1s-windows11",
    [string]$SyncDir = "hardware-results\xreal-1s-windows11",
    [switch]$RequireReady
)

$ErrorActionPreference = "Stop"

function Write-JsonFile($Path, $Value) {
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $Path) | Out-Null
    $json = $Value | ConvertTo-Json -Depth 10
    [System.IO.File]::WriteAllText(
        (Resolve-Path -LiteralPath (Split-Path -Parent $Path)).Path + [System.IO.Path]::DirectorySeparatorChar + (Split-Path -Leaf $Path),
        $json,
        [System.Text.UTF8Encoding]::new($false))
}

function Get-MatchingPnpDevices($PresentOnly) {
    $devices = if ($PresentOnly) {
        Get-PnpDevice -PresentOnly -ErrorAction SilentlyContinue
    } else {
        Get-PnpDevice -ErrorAction SilentlyContinue
    }

    $devices |
        Where-Object {
            $_.InstanceId -match "VID_3318|MRG4102" -or
            $_.FriendlyName -match "XREAL|Nreal"
        } |
        Select-Object Class, FriendlyName, InstanceId, Status, Problem
}

function Get-MonitorList($Executable) {
    if (-not $Executable -or -not (Test-Path $Executable)) {
        return [pscustomobject]@{ error = "monitor list executable not found"; path = $Executable }
    }

    $raw = & $Executable --list
    try {
        return $raw | ConvertFrom-Json
    } catch {
        return [pscustomobject]@{ error = "failed to parse monitor list"; raw = $raw }
    }
}

function Select-ExpectedMonitor($MonitorPayload, $TargetName, $DisplayNumberValue) {
    if (-not $MonitorPayload.monitors) { return $null }

    $targetDevice = "\\.\DISPLAY$DisplayNumberValue"
    $byDisplayNumber = $MonitorPayload.monitors | Where-Object { $_.device -eq $targetDevice } | Select-Object -First 1
    if ($byDisplayNumber) { return $byDisplayNumber }

    $byTargetName = $MonitorPayload.monitors |
        Where-Object {
            ($_.label -and $_.label -match $TargetName) -or
            ($_.device -and $_.device -match $TargetName)
        } |
        Select-Object -First 1
    if ($byTargetName) { return $byTargetName }

    $fallbackXrealShape = $MonitorPayload.monitors |
        Where-Object { -not $_.primary -and $_.width -eq 1920 -and $_.height -eq 1200 } |
        Select-Object -First 1
    if ($fallbackXrealShape) { return $fallbackXrealShape }

    return $MonitorPayload.monitors |
        Where-Object { -not $_.primary } |
        Select-Object -First 1
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $SyncDir | Out-Null

$presentXreal = @(Get-MatchingPnpDevices $true)
$allXreal = @(Get-MatchingPnpDevices $false)
$phantomXreal = @($allXreal | Where-Object { $_.Problem -eq 45 -or $_.Status -eq "Unknown" })
$monitorPayload = Get-MonitorList $MonitorListExecutable
$selectedMonitor = Select-ExpectedMonitor $monitorPayload $Target $DisplayNumber

$hasPresentDevice = $presentXreal.Count -gt 0
$hasWin32Monitor = $null -ne $selectedMonitor
$onlyPhantom = (-not $hasPresentDevice) -and ($phantomXreal.Count -gt 0)
$ready = $hasPresentDevice -and $hasWin32Monitor -and (-not $onlyPhantom)
$reason = if ($ready) {
    "ready"
} elseif ($onlyPhantom) {
    "xreal devices are phantom/non-present in PnP"
} elseif (-not $hasPresentDevice) {
    "xreal device is not present in PnP"
} elseif (-not $hasWin32Monitor) {
    "xreal display is not present in Win32 monitor enumeration"
} else {
    "xreal preflight failed"
}

$report = [pscustomobject]@{
    timestamp = Get-Date -Format o
    target = $Target
    displayNumber = $DisplayNumber
    monitorListExecutable = $MonitorListExecutable
    ready = $ready
    reason = $reason
    presentXrealDevices = $presentXreal
    allXrealDevices = $allXreal
    phantomXrealDevices = $phantomXreal
    win32Monitors = $monitorPayload
    selectedMonitor = $selectedMonitor
}

$reportPath = Join-Path $OutDir "xreal-preflight-report.json"
Write-JsonFile $reportPath $report
Copy-Item -Force $reportPath (Join-Path $SyncDir "xreal-preflight-report.json")

$report | ConvertTo-Json -Depth 10

if ($RequireReady -and -not $ready) {
    throw "XReal preflight failed: $reason. Do not run preview/DXGI while the device is missing or phantom-only; reboot or reseat the glasses, then rerun preflight."
}
