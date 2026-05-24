import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { createPlatformAdapter } from "../src/adapter-kit/index.js";
import { replayInputFixture, validateInputFixture } from "../src/core/input.js";
import { OverlaySession, sessionStates } from "../src/core/state-machine.js";

test("input fixtures validate and replay against a loaded session", async () => {
  const scene = JSON.parse(await readFile("fixtures/scenes/static-text.json", "utf8"));
  const fixture = JSON.parse(await readFile("fixtures/inputs/basic-actions.json", "utf8"));
  const session = new OverlaySession(createPlatformAdapter("linux"));

  assert.deepEqual(validateInputFixture(fixture), []);
  session.createSession();
  session.loadScene(scene);
  const responses = replayInputFixture(session, fixture);
  assert.equal(responses.length, 3);
  assert.equal(session.state, sessionStates.READY);
});
