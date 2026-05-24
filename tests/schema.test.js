import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { readdirSync } from "node:fs";
import path from "node:path";
import { createMessage, validateCapabilityPayload, validateCommandPayload, validateMessageEnvelope } from "../src/api/messages.js";
import { validateScene } from "../src/core/scene.js";

test("scene fixtures validate", async () => {
  const sceneDir = path.resolve("fixtures/scenes");
  for (const file of readdirSync(sceneDir).filter((name) => name.endsWith(".json"))) {
    const scene = JSON.parse(await readFile(path.join(sceneDir, file), "utf8"));
    assert.deepEqual(validateScene(scene), [], file);
  }
});

test("message envelope validates shared command messages", () => {
  const message = createMessage("command", { command: "createSession" }, {
    id: "msg-1",
    timestamp: "2026-05-24T12:00:00.000Z"
  });
  assert.deepEqual(validateMessageEnvelope(message), []);
  assert.deepEqual(validateCommandPayload(message.payload), []);
});

test("capability payload requires shared fields", () => {
  assert.deepEqual(validateCapabilityPayload({
    platform: "linux",
    display: {},
    input: [],
    permissions: {},
    unsupported: []
  }), []);
});
