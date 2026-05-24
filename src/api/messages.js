export const MESSAGE_VERSION = "0.1";

export const messageKinds = new Set([
  "command",
  "event",
  "response",
  "capability",
  "error"
]);

export const commandNames = new Set([
  "createSession",
  "loadScene",
  "updateElement",
  "removeElement",
  "input",
  "closeSession"
]);

export const inputActions = new Set([
  "select",
  "back",
  "dismiss",
  "drag",
  "pinch",
  "scroll",
  "text"
]);

export const errorCodes = {
  INVALID_MESSAGE: "invalid_message",
  INVALID_SCENE: "invalid_scene",
  UNSUPPORTED_CAPABILITY: "unsupported_capability",
  PERMISSION_REQUIRED: "permission_required",
  SESSION_NOT_READY: "session_not_ready",
  ELEMENT_NOT_FOUND: "element_not_found",
  ADAPTER_FAILURE: "adapter_failure"
};

export function createMessage(kind, payload, options = {}) {
  if (!messageKinds.has(kind)) {
    throw new Error(`Invalid message kind: ${kind}`);
  }

  return {
    version: MESSAGE_VERSION,
    kind,
    id: options.id ?? crypto.randomUUID(),
    ...(options.correlationId ? { correlationId: options.correlationId } : {}),
    timestamp: options.timestamp ?? new Date().toISOString(),
    payload
  };
}

export function createError(code, message, detail = undefined) {
  return {
    code,
    message,
    ...(detail ? { detail } : {})
  };
}

export function validateMessageEnvelope(message) {
  const errors = [];
  if (!isObject(message)) errors.push("message must be an object");
  if (errors.length) return errors;

  if (message.version !== MESSAGE_VERSION) errors.push("version must be 0.1");
  if (!messageKinds.has(message.kind)) errors.push("kind is invalid");
  if (!isNonEmptyString(message.id)) errors.push("id must be a non-empty string");
  if (!isIsoDate(message.timestamp)) errors.push("timestamp must be an ISO 8601 date-time string");
  if (!isObject(message.payload)) errors.push("payload must be an object");
  if ("correlationId" in message && !isNonEmptyString(message.correlationId)) {
    errors.push("correlationId must be a non-empty string when present");
  }
  return errors;
}

export function validateCommandPayload(payload) {
  const errors = [];
  if (!isObject(payload)) return ["payload must be an object"];
  if (!commandNames.has(payload.command)) errors.push("command is invalid");
  if (payload.command === "input") {
    if (!isObject(payload.action)) errors.push("input command requires action");
    if (payload.action && !inputActions.has(payload.action.type)) {
      errors.push("input action type is invalid");
    }
  }
  return errors;
}

export function validateCapabilityPayload(payload) {
  const errors = [];
  if (!isObject(payload)) return ["payload must be an object"];
  if (!isNonEmptyString(payload.platform)) errors.push("platform must be a non-empty string");
  if (!isObject(payload.display)) errors.push("display must be an object");
  if (!Array.isArray(payload.input)) errors.push("input must be an array");
  if (!isObject(payload.permissions)) errors.push("permissions must be an object");
  if (!Array.isArray(payload.unsupported)) errors.push("unsupported must be an array");
  return errors;
}

function isObject(value) {
  return Boolean(value) && typeof value === "object" && !Array.isArray(value);
}

function isNonEmptyString(value) {
  return typeof value === "string" && value.length > 0;
}

function isIsoDate(value) {
  if (typeof value !== "string") return false;
  const time = Date.parse(value);
  return Number.isFinite(time) && value.includes("T");
}
