import { inputActions } from "../api/messages.js";

export function validateInputFixture(fixture) {
  const errors = [];
  if (!fixture || typeof fixture !== "object" || Array.isArray(fixture)) {
    return ["fixture must be an object"];
  }
  if (fixture.version !== "0.1") errors.push("version must be 0.1");
  if (typeof fixture.id !== "string" || fixture.id.length === 0) {
    errors.push("id must be a non-empty string");
  }
  if (!Array.isArray(fixture.actions)) {
    errors.push("actions must be an array");
  } else {
    fixture.actions.forEach((action, index) => {
      if (!action || typeof action !== "object" || Array.isArray(action)) {
        errors.push(`actions[${index}] must be an object`);
      } else if (!inputActions.has(action.type)) {
        errors.push(`actions[${index}].type is invalid`);
      }
    });
  }
  return errors;
}

export function replayInputFixture(session, fixture) {
  const errors = validateInputFixture(fixture);
  if (errors.length) {
    throw new Error(`Invalid input fixture: ${errors.join("; ")}`);
  }
  return fixture.actions.map((action) => session.acceptInput(action));
}
