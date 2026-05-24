$ErrorActionPreference = "Stop"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake is required on the Windows host. Install CMake or add it to PATH, then rerun this script."
}

cmake -S platforms/windows -B build/platforms/windows
cmake --build build/platforms/windows --config Debug

powershell -ExecutionPolicy Bypass -File scripts/windows-xreal-smoke.ps1
