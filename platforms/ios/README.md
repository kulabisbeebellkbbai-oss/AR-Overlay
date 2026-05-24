# iOS Native Scaffold

This scaffold is ready for macOS/Xcode handoff. It preserves iOS 15 as the
minimum deployment target while checking newer ARKit/camera/sensor capabilities
at runtime.

This Debian host cannot run Xcode or Apple iOS Simulator. Local validation is
therefore static:

```bash
npm run verify:platform-scaffolds
grep -R "IPHONEOS_DEPLOYMENT_TARGET = 15" platforms/ios
grep -R "ARWorldTrackingConfiguration.isSupported" platforms/ios
```

Physical hardware or an Apple simulator host is required next for native iOS
runtime validation, ARKit behavior, camera/sensor access, and glasses bridge
testing.
