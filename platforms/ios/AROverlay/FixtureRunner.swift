import Foundation

enum FixtureRunner {
    static func noHardwareScene() -> OverlayScene {
        OverlayScene(id: "ios-no-hardware", width: 960, height: 540, elements: [
            OverlayElement(id: "background", type: "rect", x: 160, y: 110, width: 640, height: 280, z: 0, text: ""),
            OverlayElement(id: "headline", type: "text", x: 220, y: 178, width: 520, height: 80, z: 10, text: "AR Overlay iOS"),
            OverlayElement(id: "status", type: "text", x: 220, y: 276, width: 520, height: 52, z: 20, text: "No hardware mode")
        ])
    }
}
