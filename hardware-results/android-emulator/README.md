# Android Emulator Results

This folder is the intake point for Android emulator smoke evidence.

Run from the `android` branch:

```bash
npm run android:start-emulator
npm run android:emulator-smoke
npm run android:emulator-test
```

The start script boots the prepared `codex_api36` AVD when no ADB device is
visible. The smoke script builds the Android APK and records tooling, build,
ADB, install, launch, focus, and screenshot evidence under this folder.
`android:emulator-test` adds visual assertions and a back-input/state
transition check. These scripts do not stop or reset an emulator.
