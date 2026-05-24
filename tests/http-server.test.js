import test from "node:test";
import assert from "node:assert/strict";
import { createPlatformAdapter } from "../src/adapter-kit/index.js";
import { createMessage } from "../src/api/messages.js";
import { createOverlayHttpServer, listen } from "../src/api/http-server.js";
import { OverlaySession } from "../src/core/state-machine.js";

test("HTTP transport exposes health, capabilities, and commands", async () => {
  const session = new OverlaySession(createPlatformAdapter("linux"));
  const server = createOverlayHttpServer(session);
  const address = await listen(server);
  const base = `http://${address.address}:${address.port}`;

  try {
    const health = await fetch(`${base}/health`);
    assert.equal(health.status, 200);
    assert.equal((await health.json()).state, "idle");

    const capabilities = await fetch(`${base}/capabilities`);
    assert.equal(capabilities.status, 200);
    assert.equal((await capabilities.json()).payload.platform, "linux");

    const command = await fetch(`${base}/commands`, {
      method: "POST",
      headers: { "content-type": "application/json" },
      body: JSON.stringify(createMessage("command", { command: "createSession" }))
    });
    assert.equal(command.status, 200);
    assert.equal((await command.json()).payload.state, "ready");
  } finally {
    await new Promise((resolve) => server.close(resolve));
  }
});
