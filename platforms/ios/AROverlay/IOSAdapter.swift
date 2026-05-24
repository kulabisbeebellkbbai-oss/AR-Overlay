import Foundation

final class IOSAdapter {
    private(set) var lifecycle: [String] = []

    func createDisplaySession() {
        lifecycle.append("createDisplaySession")
    }

    func presentScene(_ scene: OverlayScene) {
        lifecycle.append("presentScene:\(scene.id)")
    }

    func translateInput(_ action: String) {
        lifecycle.append("translateInput:\(action)")
    }

    func shutdown() {
        lifecycle.append("shutdown")
    }
}
