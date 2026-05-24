import XCTest

final class FixtureReplayTests: XCTestCase {
    func testFixtureHasElements() {
        XCTAssertFalse(FixtureRunner.noHardwareScene().elements.isEmpty)
    }
}
