import { mkdir, readFile, writeFile } from "node:fs/promises";
import { readdirSync } from "node:fs";
import { createHash } from "node:crypto";
import path from "node:path";
import { renderSceneToSvg } from "../src/simulator/render-svg.js";

const sceneDir = path.resolve("fixtures/scenes");
const outDir = path.resolve("build/frames");
await mkdir(outDir, { recursive: true });
const manifest = [];

for (const file of readdirSync(sceneDir).filter((name) => name.endsWith(".json")).sort()) {
  const scene = JSON.parse(await readFile(path.join(sceneDir, file), "utf8"));
  const svg = renderSceneToSvg(scene);
  const out = path.join(outDir, file.replace(/\.json$/, ".svg"));
  await writeFile(out, svg);
  manifest.push({
    scene: file,
    frame: path.basename(out),
    sha256: createHash("sha256").update(svg).digest("hex")
  });
  console.log(out);
}

await writeFile(path.join(outDir, "manifest.json"), `${JSON.stringify(manifest, null, 2)}\n`);
