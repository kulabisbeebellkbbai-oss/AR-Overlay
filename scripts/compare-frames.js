import { readFile } from "node:fs/promises";
import { readdirSync } from "node:fs";
import { createHash } from "node:crypto";
import path from "node:path";
import { renderSceneToSvg } from "../src/simulator/render-svg.js";

const sceneDir = path.resolve("fixtures/scenes");
const expected = JSON.parse(await readFile("fixtures/expected-frames.json", "utf8"));
const actual = [];

for (const file of readdirSync(sceneDir).filter((name) => name.endsWith(".json")).sort()) {
  const scene = JSON.parse(await readFile(path.join(sceneDir, file), "utf8"));
  const svg = renderSceneToSvg(scene);
  actual.push({
    scene: file,
    frame: file.replace(/\.json$/, ".svg"),
    sha256: createHash("sha256").update(svg).digest("hex")
  });
}

if (JSON.stringify(actual, null, 2) !== JSON.stringify(expected, null, 2)) {
  console.error("Frame hash mismatch");
  console.error(JSON.stringify({ expected, actual }, null, 2));
  process.exitCode = 1;
} else {
  for (const entry of actual) {
    console.log(`${entry.frame}: ${entry.sha256}`);
  }
}
