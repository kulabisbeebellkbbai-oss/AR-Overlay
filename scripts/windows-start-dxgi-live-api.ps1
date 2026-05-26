param(
    [string]$StateFile = "build\live\windows-dxgi-scene.json",
    [string]$HostName = "127.0.0.1",
    [int]$Port = 4739
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command node -ErrorAction SilentlyContinue)) {
    throw "Node.js is required on the Windows host. Install Node.js or add it to PATH, then rerun this script."
}

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $StateFile) | Out-Null

node scripts\serve-dxgi-session.js --host=$HostName --port=$Port --stateFile=$StateFile
