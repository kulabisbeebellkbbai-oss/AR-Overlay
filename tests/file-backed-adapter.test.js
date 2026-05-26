import test from "node:test";
import assert from "node:assert/strict";
import { mkdtemp, readFile, rm } from "node:fs/promises";
import { tmpdir } from "node:os";
import path from "node:path";
import { FileBackedAdapter } from "../src/adapter-kit/file-backed.js";
import { OverlaySession } from "../src/core/state-machine.js";

test("file-backed adapter writes live DXGI scene state", async () => {
  const dir = await mkdtemp(path.join(tmpdir(), "ar-overlay-live-"));
  const stateFile = path.join(dir, "scene.json");
  try {
    const session = new OverlaySession(new FileBackedAdapter("windows", { stateFile }));
    session.createSession();
    session.loadScene({
      version: "0.1",
      id: "live-test",
      viewport: { width: 1280, height: 720 },
      elements: [{
        id: "title",
        type: "text",
        x: 0,
        y: 0,
        width: 1280,
        height: 80,
        z: 1,
        text: "Live DXGI Update"
      }]
    });

    const state = JSON.parse(await readFile(stateFile, "utf8"));
    assert.equal(state.mode, "dxgi-live-state");
    assert.equal(state.state, "sceneLoaded");
    assert.equal(state.sceneId, "live-test");
    assert.equal(state.primaryText, "Live DXGI Update");
    assert.equal(state.scene.elements[0].text, "Live DXGI Update");
  } finally {
    await rm(dir, { recursive: true, force: true });
  }
});
