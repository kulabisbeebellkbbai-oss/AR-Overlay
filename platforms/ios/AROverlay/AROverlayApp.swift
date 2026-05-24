import SwiftUI

@main
struct AROverlayApp: App {
    var body: some Scene {
        WindowGroup {
            OverlayView(scene: FixtureRunner.noHardwareScene())
        }
    }
}
