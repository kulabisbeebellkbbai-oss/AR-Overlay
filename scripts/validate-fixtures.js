import { readFile } from "node:fs/promises";
import { readdirSync } from "node:fs";
import path from "node:path";
import { validateScene } from "../src/core/scene.js";
import { validateInputFixture } from "../src/core/input.js";

const sceneDir = path.resolve("fixtures/scenes");
const inputDir = path.resolve("fixtures/inputs");
let failures = 0;

for (const file of readdirSync(sceneDir).filter((name) => name.endsWith(".json")).sort()) {
  const scene = JSON.parse(await readFile(path.join(sceneDir, file), "utf8"));
  const errors = validateScene(scene);
  if (errors.length) {
    failures += 1;
    console.error(`${file}: ${errors.join("; ")}`);
  } else {
    console.log(`${file}: ok`);
  }
}

for (const file of readdirSync(inputDir).filter((name) => name.endsWith(".json")).sort()) {
  const fixture = JSON.parse(await readFile(path.join(inputDir, file), "utf8"));
  const errors = validateInputFixture(fixture);
  if (errors.length) {
    failures += 1;
    console.error(`${file}: ${errors.join("; ")}`);
  } else {
    console.log(`${file}: ok`);
  }
}

if (failures) {
  process.exitCode = 1;
}
