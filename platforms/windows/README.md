# Windows Native Scaffold

This scaffold provides a dependency-light C++ adapter smoke binary. It can be
configured with CMake on this Debian host to validate code shape; true Windows
11 runtime validation requires a Windows host or VM image.

## Local Verification

```bash
cmake -S platforms/windows -B build/platforms/windows-host-check
cmake --build build/platforms/windows-host-check
build/platforms/windows-host-check/ar-overlay-windows-smoke
```

Physical hardware or a Windows 11 runtime is required next for Win32/WinUI
graphics behavior, drivers, camera/sensor access, and AR/XR glasses bridge
testing.
