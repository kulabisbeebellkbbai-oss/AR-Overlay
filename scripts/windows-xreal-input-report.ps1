param(
    [int]$ObservationSeconds = 20,
    [string]$OutDir = "build\hardware\xreal-1s-windows11",
    [string]$SyncDir = "hardware-results\xreal-1s-windows11"
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

function Select-PnpSummary {
    param(
        [Parameter(ValueFromPipeline = $true)]
        $Device
    )

    process {
        $Device | Select-Object Class, FriendlyName, InstanceId, Status, Problem
    }
}

function Get-XrealPnpDevices {
    Get-PnpDevice -ErrorAction SilentlyContinue |
        Where-Object {
            $_.InstanceId -match "VID_3318|PID_043E|MRG4102" -or
            $_.FriendlyName -match "XREAL|Nreal"
        } |
        Select-PnpSummary
}

function Get-InputClassDevices {
    $classes = @("HIDClass", "Keyboard", "Mouse", "MEDIA", "AudioEndpoint", "USB", "Net")
    foreach ($class in $classes) {
        Get-PnpDevice -Class $class -ErrorAction SilentlyContinue |
            Where-Object {
                $_.InstanceId -match "VID_3318|PID_043E|MRG4102" -or
                $_.FriendlyName -match "XREAL|Nreal|HID-compliant|USB Input|Consumer Control"
            } |
            Select-PnpSummary
    }
}

function Get-XrealCimDevices {
    Get-CimInstance Win32_PnPEntity -ErrorAction SilentlyContinue |
        Where-Object {
            $_.DeviceID -match "VID_3318|PID_043E|MRG4102" -or
            $_.Name -match "XREAL|Nreal"
        } |
        Select-Object Name, PNPClass, DeviceID, Manufacturer, Service, Status, ConfigManagerErrorCode
}

function Get-KeyboardDevices {
    Get-CimInstance Win32_Keyboard -ErrorAction SilentlyContinue |
        Select-Object Name, DeviceID, PNPDeviceID, Status
}

function Get-PointingDevices {
    Get-CimInstance Win32_PointingDevice -ErrorAction SilentlyContinue |
        Select-Object Name, DeviceID, PNPDeviceID, HardwareType, Status
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
New-Item -ItemType Directory -Force -Path $SyncDir | Out-Null

$timestamp = Get-Date -Format o
$before = [pscustomobject]@{
    xrealPnp = @(Get-XrealPnpDevices)
    xrealCim = @(Get-XrealCimDevices)
    inputClassDevices = @(Get-InputClassDevices)
    keyboards = @(Get-KeyboardDevices)
    pointingDevices = @(Get-PointingDevices)
}

Write-Host "Collecting XReal input discovery report."
Write-Host "For the next $ObservationSeconds seconds, press or use any XReal controls that should produce Windows input."
Start-Sleep -Seconds $ObservationSeconds

$after = [pscustomobject]@{
    xrealPnp = @(Get-XrealPnpDevices)
    xrealCim = @(Get-XrealCimDevices)
    inputClassDevices = @(Get-InputClassDevices)
    keyboards = @(Get-KeyboardDevices)
    pointingDevices = @(Get-PointingDevices)
}

$report = [pscustomobject]@{
    timestamp = $timestamp
    observationSeconds = $ObservationSeconds
    host = [pscustomobject]@{
        computerName = $env:COMPUTERNAME
        os = (Get-CimInstance Win32_OperatingSystem -ErrorAction SilentlyContinue |
            Select-Object Caption, Version, BuildNumber)
    }
    before = $before
    after = $after
    captureLimitations = @(
        "This PowerShell report captures Windows device presence and state before and after the observation window.",
        "It does not decode HID event payloads. If XReal controls appear only as HID reports, the next step is a native Raw Input/HID listener."
    )
}

$reportPath = Join-Path $OutDir "xreal-input-report.json"
Write-JsonFile $reportPath $report

$manualPath = Join-Path $OutDir "input-manual-result.md"
$manual = @"
# XReal 1S Windows Input Manual Result

Collected: $timestamp

During the input observation window, press or use any XReal controls that should
produce Windows input.

## Observations

- Any XReal control physically available: TODO
- Any Windows keyboard/mouse/media event observed: TODO
- Any visible overlay reaction expected but missing: TODO
- Shared action mapping candidate: TODO
- Notes: TODO

## Result

- Input path usable for AR Overlay shared controls: TODO
- If not usable, limitation to record: TODO
"@
[System.IO.File]::WriteAllText(
    (Resolve-Path -LiteralPath (Split-Path -Parent $manualPath)).Path + [System.IO.Path]::DirectorySeparatorChar + (Split-Path -Leaf $manualPath),
    $manual,
    [System.Text.UTF8Encoding]::new($false))

Copy-Item -Force $reportPath (Join-Path $SyncDir "xreal-input-report.json")
Copy-Item -Force $manualPath (Join-Path $SyncDir "input-manual-result.md")

Write-Host "Wrote $reportPath"
Write-Host "Copied input evidence to $SyncDir"
Write-Host "Complete hardware-results\xreal-1s-windows11\input-manual-result.md before syncing results."
