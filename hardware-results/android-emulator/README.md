# Android Emulator Results

This folder is the intake point for Android emulator smoke evidence.

Run from the `android` branch:

```bash
npm run android:emulator-smoke
```

The script builds the Android APK and records tooling, build, ADB, install, and
launch evidence under this folder. It does not start, stop, reset, or otherwise
manage an emulator; install and launch only run when an ADB device is already
visible.
