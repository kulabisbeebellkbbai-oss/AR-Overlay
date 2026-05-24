import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { renderSceneToSvg } from "../src/simulator/render-svg.js";

test("simulator renders deterministic SVG in z order", async () => {
  const scene = JSON.parse(await readFile("fixtures/scenes/z-order.json", "utf8"));
  const svg = renderSceneToSvg(scene);
  assert.match(svg, /<svg/);
  assert.ok(svg.indexOf("background") < svg.indexOf("foreground"));
});

test("simulator escapes text content", () => {
  const svg = renderSceneToSvg({
    version: "0.1",
    id: "escape",
    viewport: { width: 100, height: 100 },
    elements: [{
      id: "text",
      type: "text",
      x: 0,
      y: 0,
      width: 100,
      height: 20,
      z: 0,
      text: "<ready>",
      fill: "#ffffff"
    }]
  });
  assert.match(svg, /&lt;ready&gt;/);
});
