import test from "node:test";
import assert from "node:assert/strict";
import { mkdtemp, readFile, rm } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import { loadScenario, runPlatformScenario, supportedEmulatedPlatforms } from "../src/emulator/platform-emulator.js";

test("platform emulators run the full no-hardware scenario", async () => {
  const temp = await mkdtemp(path.join(os.tmpdir(), "ar-overlay-emulator-"));
  const scenario = await loadScenario("fixtures/scenarios/full-session.json");

  try {
    for (const platform of supportedEmulatedPlatforms) {
      const transcript = await runPlatformScenario(platform, scenario, {
        outDir: path.join(temp, platform)
      });
      assert.equal(transcript.platform, platform);
      assert.equal(transcript.exchanges.length, scenario.steps.length);
      assert.equal(transcript.exchanges.every((exchange) => exchange.response.kind === "response"), true);
      assert.equal(transcript.frames.length, 3);

      const saved = JSON.parse(await readFile(path.join(temp, platform, "transcript.json"), "utf8"));
      assert.equal(saved.diagnostics.mode, "no-hardware");
    }
  } finally {
    await rm(temp, { recursive: true, force: true });
  }
});
