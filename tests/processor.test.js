import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { createPlatformAdapter } from "../src/adapter-kit/index.js";
import { createMessage } from "../src/api/messages.js";
import { processMessage } from "../src/api/processor.js";
import { OverlaySession } from "../src/core/state-machine.js";

test("processor dispatches shared command messages", async () => {
  const scene = JSON.parse(await readFile("fixtures/scenes/static-text.json", "utf8"));
  const session = new OverlaySession(createPlatformAdapter("linux"));

  const create = processMessage(session, createMessage("command", { command: "createSession" }));
  assert.equal(create.kind, "response");
  assert.equal(create.payload.ok, true);

  const load = processMessage(session, createMessage("command", { command: "loadScene", scene }));
  assert.equal(load.kind, "response");
  assert.equal(load.payload.sceneId, "static-text");
});

test("processor rejects invalid command messages", () => {
  const session = new OverlaySession(createPlatformAdapter("linux"));
  const response = processMessage(session, createMessage("command", { command: "bogus" }));
  assert.equal(response.kind, "error");
  assert.equal(response.payload.code, "invalid_message");
});
