import { createMessage, validateCommandPayload, validateMessageEnvelope } from "./messages.js";

export function processMessage(session, message) {
  const envelopeErrors = validateMessageEnvelope(message);
  if (envelopeErrors.length) {
    return createMessage("error", {
      code: "invalid_message",
      message: "Invalid message envelope",
      detail: { errors: envelopeErrors }
    }, { correlationId: message?.id });
  }

  if (message.kind !== "command") {
    return createMessage("error", {
      code: "invalid_message",
      message: "Only command messages can be processed"
    }, { correlationId: message.id });
  }

  const commandErrors = validateCommandPayload(message.payload);
  if (commandErrors.length) {
    return createMessage("error", {
      code: "invalid_message",
      message: "Invalid command payload",
      detail: { errors: commandErrors }
    }, { correlationId: message.id });
  }

  try {
    const payload = dispatchCommand(session, message.payload);
    return createMessage("response", payload, { correlationId: message.id });
  } catch (error) {
    return createMessage("error", {
      code: "adapter_failure",
      message: error.message
    }, { correlationId: message.id });
  }
}

function dispatchCommand(session, payload) {
  switch (payload.command) {
    case "createSession":
      return session.createSession();
    case "loadScene":
      return session.loadScene(payload.scene);
    case "updateElement":
      return session.updateElement(payload.element);
    case "removeElement":
      return session.removeElement(payload.elementId);
    case "input":
      return session.acceptInput(payload.action);
    case "closeSession":
      return session.closeSession();
    default:
      throw new Error(`Unsupported command: ${payload.command}`);
  }
}
