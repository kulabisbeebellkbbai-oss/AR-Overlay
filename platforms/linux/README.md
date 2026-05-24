# Linux Native Scaffold

This scaffold provides a no-hardware native smoke binary for the Linux hardware
branch. It does not bind to AR/XR glasses yet; it exercises the shared lifecycle
boundary and emits deterministic capability/diagnostic JSON.

## Local Verification

```bash
cmake -S platforms/linux -B build/platforms/linux
cmake --build build/platforms/linux
build/platforms/linux/ar-overlay-linux-smoke
```

Physical hardware is required next for real display bridge discovery, compositor
presentation timing, sensors, and glasses input.
