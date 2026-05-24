import test from "node:test";
import assert from "node:assert/strict";
import { createPlatformAdapter } from "../src/adapter-kit/index.js";

for (const platform of ["linux", "android", "ios", "windows"]) {
  test(`${platform} no-hardware adapter reports shared capabilities`, () => {
    const adapter = createPlatformAdapter(platform);
    const report = adapter.capabilityReport();
    assert.equal(report.platform, platform);
    assert.equal(report.display.mode, "simulated");
    assert.ok(report.input.includes("select"));
    assert.ok(report.unsupported.some((item) => item.feature === "physicalDisplay"));

    const session = adapter.createDisplaySession();
    assert.equal(session.ok, true);
    assert.equal(adapter.shutdown().ok, true);
    assert.ok(adapter.diagnostics().lifecycle.includes("shutdown"));
  });
}
