import { createHash } from "node:crypto";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import path from "node:path";
import { createPlatformAdapter } from "../adapter-kit/index.js";
import { createMessage } from "../api/messages.js";
import { processMessage } from "../api/processor.js";
import { OverlaySession } from "../core/state-machine.js";
import { renderSceneToSvg } from "../simulator/render-svg.js";

export const supportedEmulatedPlatforms = ["linux", "android", "ios", "windows"];

export async function runPlatformScenario(platform, scenario, options = {}) {
  if (!supportedEmulatedPlatforms.includes(platform)) {
    throw new Error(`Unsupported emulated platform: ${platform}`);
  }

  const outDir = path.resolve(options.outDir ?? `build/emulators/${platform}`);
  await mkdir(outDir, { recursive: true });

  const adapter = createPlatformAdapter(platform);
  const session = new OverlaySession(adapter);
  const transcript = {
    version: "0.1",
    platform,
    scenario: scenario.id,
    capabilities: session.capabilityReport(),
    exchanges: [],
    frames: [],
    diagnostics: null
  };

  for (const step of scenario.steps) {
    const message = createMessage("command", step.payload, {
      id: `${scenario.id}-${platform}-${step.id}`,
      timestamp: step.timestamp
    });
    const result = processMessage(session, message);
    transcript.exchanges.push({
      step: step.id,
      request: message,
      response: result
    });

    if (session.scene) {
      const svg = renderSceneToSvg(session.scene);
      const frameName = `${String(transcript.frames.length + 1).padStart(2, "0")}-${step.id}.svg`;
      await writeFile(path.join(outDir, frameName), svg);
      transcript.frames.push({
        step: step.id,
        frame: frameName,
        sha256: createHash("sha256").update(svg).digest("hex")
      });
    }
  }

  transcript.diagnostics = adapter.diagnostics();
  await writeFile(path.join(outDir, "capabilities.json"), `${JSON.stringify(transcript.capabilities, null, 2)}\n`);
  await writeFile(path.join(outDir, "diagnostics.json"), `${JSON.stringify(transcript.diagnostics, null, 2)}\n`);
  await writeFile(path.join(outDir, "transcript.json"), `${JSON.stringify(transcript, null, 2)}\n`);
  return transcript;
}

export async function loadScenario(file) {
  return JSON.parse(await readFile(file, "utf8"));
}
