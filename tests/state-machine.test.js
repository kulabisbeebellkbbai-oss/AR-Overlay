import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { createPlatformAdapter } from "../src/adapter-kit/index.js";
import { OverlaySession, sessionStates } from "../src/core/state-machine.js";

test("session loads a scene and accepts dismiss input", async () => {
  const scene = JSON.parse(await readFile("fixtures/scenes/static-text.json", "utf8"));
  const session = new OverlaySession(createPlatformAdapter("linux"));

  assert.equal(session.state, sessionStates.IDLE);
  assert.equal(session.createSession().state, sessionStates.READY);
  assert.equal(session.loadScene(scene).state, sessionStates.SCENE_LOADED);
  assert.equal(session.acceptInput({ type: "dismiss" }).state, sessionStates.READY);
});

test("session can update and remove elements", async () => {
  const scene = JSON.parse(await readFile("fixtures/scenes/static-text.json", "utf8"));
  const session = new OverlaySession(createPlatformAdapter("linux"));

  session.createSession();
  session.loadScene(scene);
  session.updateElement({
    id: "badge",
    type: "rect",
    x: 10,
    y: 10,
    width: 30,
    height: 30,
    z: 1,
    fill: "#ffffff"
  });
  assert.equal(session.scene.elements.some((element) => element.id === "badge"), true);
  session.removeElement("badge");
  assert.equal(session.scene.elements.some((element) => element.id === "badge"), false);
});
