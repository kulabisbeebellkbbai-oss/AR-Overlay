import { readFile } from "node:fs/promises";
import { createMessage } from "../src/api/messages.js";

const options = parseArgs(process.argv.slice(2));
const base = options.base ?? "http://127.0.0.1:4739";
const scenePath = options.scene ?? "fixtures/scenes/static-text.json";
const scene = JSON.parse(await readFile(scenePath, "utf8"));

for (const payload of [
  { command: "createSession" },
  { command: "loadScene", scene }
]) {
  const response = await fetch(`${base}/commands`, {
    method: "POST",
    headers: { "content-type": "application/json" },
    body: JSON.stringify(createMessage("command", payload))
  });
  const body = await response.json();
  console.log(JSON.stringify(body, null, 2));
  if (!response.ok || body.kind === "error") process.exitCode = 1;
}

function parseArgs(args) {
  const parsed = {};
  for (const arg of args) {
    if (!arg.startsWith("--")) continue;
    const [key, value = "true"] = arg.slice(2).split("=");
    parsed[key] = value;
  }
  return parsed;
}
