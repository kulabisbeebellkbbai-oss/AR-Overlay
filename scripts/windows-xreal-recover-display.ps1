param(
    [string]$OutDir = "build\hardware\xreal-1s-windows11",
    [switch]$AdminRescan,
    [switch]$OpenDisplaySettings,
    [switch]$TryExtend
)

$ErrorActionPreference = "Stop"

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Write-JsonFile($Path, $Value) {
    $json = $Value | ConvertTo-Json -Depth 8
    [System.IO.File]::WriteAllText(
        (Resolve-Path -LiteralPath (Split-Path -Parent $Path)).Path + [System.IO.Path]::DirectorySeparatorChar + (Split-Path -Leaf $Path),
        $json,
        [System.Text.UTF8Encoding]::new($false))
}

function Invoke-DisplayTopology($Name, $Flag) {
    $source = @"
using System;
using System.Runtime.InteropServices;
public class AROverlayDisplayConfig {
  [DllImport("user32.dll", SetLastError=true)]
  public static extern int SetDisplayConfig(
    uint numPathArrayElements,
    IntPtr pathArray,
    uint numModeInfoArrayElements,
    IntPtr modeInfoArray,
    uint flags);
}
"@
    if (-not ([System.Management.Automation.PSTypeName]"AROverlayDisplayConfig").Type) {
        Add-Type $source
    }

    $sdcApply = 0x80
    $result = [AROverlayDisplayConfig]::SetDisplayConfig(
        0,
        [IntPtr]::Zero,
        0,
        [IntPtr]::Zero,
        $sdcApply -bor $Flag)

    [pscustomobject]@{
        topology = $Name
        result = $result
        resultMeaning = switch ($result) {
            0 { "success" }
            31 { "ERROR_GEN_FAILURE" }
            87 { "ERROR_INVALID_PARAMETER" }
            1223 { "ERROR_CANCELLED" }
            default { "Win32 error $result" }
        }
    }
}

function Get-Win32MonitorList {
    $exe = "build\platforms\windows\Debug\ar-overlay-windows-preview.exe"
    if (-not (Test-Path $exe)) {
        return [pscustomobject]@{ error = "preview binary not found"; path = $exe }
    }

    $raw = & $exe --list
    try {
        return $raw | ConvertFrom-Json
    } catch {
        return [pscustomobject]@{ error = "failed to parse monitor list"; raw = $raw }
    }
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$isAdmin = Test-IsAdministrator
if ($AdminRescan -and -not $isAdmin) {
    $script = $PSCommandPath
    $args = @(
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", "`"$script`"",
        "-OutDir", "`"$OutDir`"",
        "-AdminRescan"
    )
    Start-Process powershell -Verb RunAs -ArgumentList $args
    Write-Host "Started elevated PnP rescan. Accept the UAC prompt, then rerun this script without -AdminRescan."
    exit 0
}

if ($AdminRescan) {
    pnputil /scan-devices
    Start-Sleep -Seconds 5
}

if ($OpenDisplaySettings) {
    Start-Process "ms-settings:display"
}

$timestamp = Get-Date -Format o
$pnpXreal = Get-PnpDevice |
    Where-Object {
        $_.InstanceId -match "VID_3318|MRG4102" -or
        $_.FriendlyName -match "XREAL|Nreal"
    } |
    Select-Object Class, FriendlyName, InstanceId, Status, Problem

$monitorDevices = Get-PnpDevice -Class Monitor |
    Select-Object FriendlyName, InstanceId, Status, Problem

$desktopMonitors = Get-CimInstance Win32_DesktopMonitor |
    Select-Object Name, PNPDeviceID, ScreenWidth, ScreenHeight, Status

$wmiMonitorIds = Get-CimInstance -Namespace root\wmi -ClassName WmiMonitorID |
    ForEach-Object {
        $name = ($_.UserFriendlyName |
            Where-Object { $_ -ne 0 } |
            ForEach-Object { [char]$_ }) -join ""
        [pscustomobject]@{
            InstanceName = $_.InstanceName
            Name = $name
            Active = $_.Active
        }
    }

$wmiConnections = Get-CimInstance -Namespace root\wmi -ClassName WmiMonitorConnectionParams |
    Select-Object InstanceName, Active, VideoOutputTechnology

$wmiBasic = Get-CimInstance -Namespace root\wmi -ClassName WmiMonitorBasicDisplayParams |
    Select-Object InstanceName, Active, MaxHorizontalImageSize, MaxVerticalImageSize

$win32Monitors = Get-Win32MonitorList
$topologyResults = @()
$win32MonitorsAfterExtend = $null
if ($TryExtend) {
    $topologyResults = @(
        Invoke-DisplayTopology "extend" 0x4
    )
    Start-Sleep -Seconds 2
    $win32MonitorsAfterExtend = Get-Win32MonitorList
}

$report = [pscustomobject]@{
    timestamp = $timestamp
    isAdministrator = $isAdmin
    pnpXreal = $pnpXreal
    monitorDevices = $monitorDevices
    desktopMonitors = $desktopMonitors
    wmiMonitorIds = $wmiMonitorIds
    wmiConnections = $wmiConnections
    wmiBasicDisplayParams = $wmiBasic
    win32MonitorsBeforeExtend = $win32Monitors
    tryExtendRequested = [bool]$TryExtend
    topologyResults = $topologyResults
    win32MonitorsAfterExtend = $win32MonitorsAfterExtend
}

$reportPath = Join-Path $OutDir "xreal-display-recovery-report.json"
Write-JsonFile $reportPath $report

Write-Host "Wrote $reportPath"
if ($TryExtend) {
    Write-Host "Win32 monitors after extend attempt:"
    $win32MonitorsAfterExtend | ConvertTo-Json -Depth 8
} else {
    Write-Host "Win32 monitors:"
    $win32Monitors | ConvertTo-Json -Depth 8
    Write-Host "No display topology changes attempted. Rerun with -TryExtend to request Extend."
}
