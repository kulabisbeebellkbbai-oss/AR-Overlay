import ARKit
import Foundation

struct IOSCapabilityProbe {
    static func capabilityReport() -> [String: Any] {
        [
            "platform": "ios",
            "display": [
                "mode": "native-preview",
                "minimumDeploymentTarget": "ios-15"
            ],
            "input": ["select", "back", "dismiss", "drag", "pinch", "scroll", "text"],
            "permissions": [
                "hardwareDisplay": "notRequired",
                "arkit": ARWorldTrackingConfiguration.isSupported ? "available" : "unsupported"
            ],
            "sensors": [],
            "hardwareId": NSNull(),
            "unsupported": [
                ["feature": "physicalDisplay", "reason": "no_hardware_mode"]
            ]
        ]
    }
}
