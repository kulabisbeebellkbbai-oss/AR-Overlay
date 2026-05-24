# Hardware Branches

AR Overlay will keep hardware-specific work on dedicated long-lived branches.
Each branch should carry the minimum platform-specific code needed for that
target, while reusable overlay concepts remain on `main`.

## Branches

| Branch | Target | Scope |
| --- | --- | --- |
| `linux` | Linux AR/XR host support | Native desktop and embedded Linux display paths |
| `android` | Android AR/XR host support | Android device integration, permissions, and display services |
| `ios` | iOS AR/XR host support | iOS device integration, entitlement notes, and display services |
| `windows` | Windows AR/XR host support | Windows display paths, drivers, and host integration |

## Working Rule

Start shared behavior on `main`. Move only platform-specific implementation,
configuration, and hardware notes to the relevant hardware branch.
