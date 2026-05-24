import { normalizeScene } from "../core/scene.js";

export function renderSceneToSvg(scene) {
  const normalized = normalizeScene(scene);
  const width = normalized.viewport.width;
  const height = normalized.viewport.height;
  const elements = normalized.elements.map(renderElement).join("\n  ");

  return [
    `<?xml version="1.0" encoding="UTF-8"?>`,
    `<svg xmlns="http://www.w3.org/2000/svg" width="${width}" height="${height}" viewBox="0 0 ${width} ${height}" role="img" aria-label="${escapeXml(normalized.id)}">`,
    `  <rect x="0" y="0" width="${width}" height="${height}" fill="#000000" opacity="0"/>`,
    `  ${elements}`,
    `</svg>`,
    ""
  ].join("\n");
}

function renderElement(element) {
  if (element.type === "rect") return renderRect(element);
  if (element.type === "text") return renderText(element);
  if (element.type === "image") return renderImage(element);
  throw new Error(`Unsupported element type: ${element.type}`);
}

function renderRect(element) {
  return `<rect id="${escapeXml(element.id)}" x="${element.x}" y="${element.y}" width="${element.width}" height="${element.height}" fill="${element.fill ?? "#ffffff"}" opacity="${element.opacity ?? 1}"/>`;
}

function renderText(element) {
  const align = element.align ?? "start";
  const anchor = align === "center" ? "middle" : align === "end" ? "end" : "start";
  const x = align === "center" ? element.x + element.width / 2 : align === "end" ? element.x + element.width : element.x;
  const y = element.y + (element.fontSize ?? 24);
  return `<text id="${escapeXml(element.id)}" x="${x}" y="${y}" width="${element.width}" height="${element.height}" fill="${element.fill ?? "#ffffff"}" font-family="${escapeXml(element.fontFamily ?? "system-ui")}" font-size="${element.fontSize ?? 24}" text-anchor="${anchor}">${escapeXml(element.text)}</text>`;
}

function renderImage(element) {
  return `<image id="${escapeXml(element.id)}" href="${escapeXml(element.href)}" x="${element.x}" y="${element.y}" width="${element.width}" height="${element.height}" preserveAspectRatio="${escapeXml(element.preserveAspectRatio ?? "xMidYMid meet")}"/>`;
}

function escapeXml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("\"", "&quot;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}
