import { mkdir, writeFile } from "node:fs/promises";
import path from "node:path";
import { loadScenario, runPlatformScenario, supportedEmulatedPlatforms } from "../src/emulator/platform-emulator.js";

const args = new Set(process.argv.slice(2));
const scenarioPath = path.resolve("fixtures/scenarios/full-session.json");
const outDir = path.resolve("build/emulators");
const scenario = await loadScenario(scenarioPath);
await mkdir(outDir, { recursive: true });

const platforms = args.has("--all")
  ? supportedEmulatedPlatforms
  : supportedEmulatedPlatforms.filter((platform) => args.has(`--${platform}`));

const selected = platforms.length ? platforms : supportedEmulatedPlatforms;
const summary = [];

for (const platform of selected) {
  const transcript = await runPlatformScenario(platform, scenario, {
    outDir: path.join(outDir, platform)
  });
  summary.push({
    platform,
    scenario: transcript.scenario,
    exchanges: transcript.exchanges.length,
    frames: transcript.frames.length,
    finalState: transcript.exchanges.at(-1)?.response.payload?.state ?? null
  });
  console.log(`${platform}: ${transcript.exchanges.length} exchanges, ${transcript.frames.length} frames`);
}

await writeFile(path.join(outDir, "summary.json"), `${JSON.stringify(summary, null, 2)}\n`);
