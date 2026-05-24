import XCTest

final class CapabilityProbeTests: XCTestCase {
    func testCapabilityReportContainsSharedPlatform() {
        let report = IOSCapabilityProbe.capabilityReport()
        XCTAssertEqual(report["platform"] as? String, "ios")
    }
}
