import { createPlatformAdapter } from "../src/adapter-kit/index.js";
import { createOverlayHttpServer, listen } from "../src/api/http-server.js";
import { OverlaySession } from "../src/core/state-machine.js";

const platformArg = process.argv.find((arg) => arg.startsWith("--platform="));
const portArg = process.argv.find((arg) => arg.startsWith("--port="));
const platform = platformArg ? platformArg.split("=")[1] : "linux";
const port = portArg ? Number(portArg.split("=")[1]) : 3000;
const session = new OverlaySession(createPlatformAdapter(platform));
const server = createOverlayHttpServer(session);
const address = await listen(server, { port });

console.log(`AR Overlay API listening on http://${address.address}:${address.port} for ${platform}`);
