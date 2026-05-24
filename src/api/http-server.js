import http from "node:http";
import { createMessage } from "./messages.js";
import { processMessage } from "./processor.js";

export function createOverlayHttpServer(session) {
  return http.createServer(async (request, response) => {
    try {
      if (request.method === "GET" && request.url === "/health") {
        return sendJson(response, 200, { ok: true, state: session.state });
      }

      if (request.method === "GET" && request.url === "/capabilities") {
        return sendJson(response, 200, createMessage("capability", session.capabilityReport()));
      }

      if (request.method === "POST" && request.url === "/commands") {
        const body = await readJson(request);
        const result = processMessage(session, body);
        return sendJson(response, result.kind === "error" ? 400 : 200, result);
      }

      sendJson(response, 404, {
        ok: false,
        error: {
          code: "not_found",
          message: "Route not found"
        }
      });
    } catch (error) {
      sendJson(response, 500, {
        ok: false,
        error: {
          code: "server_error",
          message: error.message
        }
      });
    }
  });
}

export function listen(server, options = {}) {
  const host = options.host ?? "127.0.0.1";
  const port = options.port ?? 0;
  return new Promise((resolve, reject) => {
    server.once("error", reject);
    server.listen(port, host, () => {
      server.off("error", reject);
      resolve(server.address());
    });
  });
}

async function readJson(request) {
  const chunks = [];
  for await (const chunk of request) chunks.push(chunk);
  const text = Buffer.concat(chunks).toString("utf8");
  return JSON.parse(text);
}

function sendJson(response, statusCode, payload) {
  const body = JSON.stringify(payload, null, 2);
  response.writeHead(statusCode, {
    "content-type": "application/json; charset=utf-8",
    "content-length": Buffer.byteLength(body)
  });
  response.end(body);
}
