export const elementTypes = new Set(["text", "image", "rect"]);

export function validateScene(scene) {
  const errors = [];
  if (!isObject(scene)) return ["scene must be an object"];
  if (scene.version !== "0.1") errors.push("scene.version must be 0.1");
  if (!isNonEmptyString(scene.id)) errors.push("scene.id must be a non-empty string");
  if (!isObject(scene.viewport)) {
    errors.push("scene.viewport must be an object");
  } else {
    if (!isPositiveNumber(scene.viewport.width)) errors.push("viewport.width must be positive");
    if (!isPositiveNumber(scene.viewport.height)) errors.push("viewport.height must be positive");
  }
  if (!Array.isArray(scene.elements)) {
    errors.push("scene.elements must be an array");
  } else {
    const ids = new Set();
    scene.elements.forEach((element, index) => {
      for (const error of validateElement(element)) {
        errors.push(`elements[${index}].${error}`);
      }
      if (element && ids.has(element.id)) errors.push(`elements[${index}].id must be unique`);
      if (element) ids.add(element.id);
    });
  }
  return errors;
}

export function normalizeScene(scene) {
  const errors = validateScene(scene);
  if (errors.length) {
    throw new Error(`Invalid scene: ${errors.join("; ")}`);
  }

  return {
    ...scene,
    elements: [...scene.elements].sort((a, b) => a.z - b.z || a.id.localeCompare(b.id))
  };
}

export function validateElement(element) {
  const errors = [];
  if (!isObject(element)) return ["must be an object"];
  if (!isNonEmptyString(element.id)) errors.push("id must be a non-empty string");
  if (!elementTypes.has(element.type)) errors.push("type is invalid");
  for (const key of ["x", "y", "width", "height", "z"]) {
    if (typeof element[key] !== "number" || !Number.isFinite(element[key])) {
      errors.push(`${key} must be a finite number`);
    }
  }
  if (element.width <= 0) errors.push("width must be positive");
  if (element.height <= 0) errors.push("height must be positive");
  if (element.type === "text" && !isNonEmptyString(element.text)) {
    errors.push("text elements require text");
  }
  if (element.type === "image" && !isNonEmptyString(element.href)) {
    errors.push("image elements require href");
  }
  return errors;
}

function isObject(value) {
  return Boolean(value) && typeof value === "object" && !Array.isArray(value);
}

function isNonEmptyString(value) {
  return typeof value === "string" && value.length > 0;
}

function isPositiveNumber(value) {
  return typeof value === "number" && Number.isFinite(value) && value > 0;
}
