import { createOverlayHttpServer, listen } from "../src/api/http-server.js";
import { FileBackedAdapter } from "../src/adapter-kit/file-backed.js";
import { OverlaySession } from "../src/core/state-machine.js";

const options = parseArgs(process.argv.slice(2));
const stateFile = options.stateFile ?? "build/live/windows-dxgi-scene.json";
const host = options.host ?? "127.0.0.1";
const port = Number(options.port ?? 4739);

const adapter = new FileBackedAdapter("windows", {
  stateFile,
  overrides: {
    display: {
      osFamily: "windows-11",
      graphicsBackend: "dxgi-live-state-file"
    }
  }
});
const session = new OverlaySession(adapter);
const server = createOverlayHttpServer(session);
const address = await listen(server, { host, port });

console.log(JSON.stringify({
  ok: true,
  mode: "dxgi-live-session",
  host: address.address,
  port: address.port,
  stateFile
}, null, 2));

function parseArgs(args) {
  const parsed = {};
  for (const arg of args) {
    if (!arg.startsWith("--")) continue;
    const [key, value = "true"] = arg.slice(2).split("=");
    parsed[key] = value;
  }
  return parsed;
}
