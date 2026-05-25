import { existsSync, readFileSync } from "node:fs";
import path from "node:path";
import zlib from "node:zlib";

const mode = argValue("--mode") ?? "initial";
const file = argValue("--screenshot")
  ?? (mode === "dismissed" ? "hardware-results/android-emulator/back-screen.png" : "hardware-results/android-emulator/screen.png");
const focusFile = argValue("--focus") ?? "hardware-results/android-emulator/window-focus.txt";
const packageName = "com.kulabisbeebellkbbai.aroverlay";

if (!existsSync(file)) fail(`missing screenshot: ${file}`);
if (!existsSync(focusFile)) fail(`missing focus evidence: ${focusFile}`);

const focus = readFileSync(focusFile, "utf8");
if (!focus.includes(packageName)) fail(`focus evidence does not include ${packageName}`);

const png = decodePng(readFileSync(file));
if (png.width < 800 || png.height < 1200) {
  fail(`unexpected screenshot dimensions ${png.width}x${png.height}`);
}

const center = averageRegion(png, 0.42, 0.42, 0.58, 0.58);
const topLeft = averageRegion(png, 0.05, 0.05, 0.15, 0.10);

if (!isLight(topLeft)) {
  fail(`expected light Android app background near top-left, got ${colorLabel(topLeft)}`);
}

if (mode === "initial") {
  if (!isDark(center)) fail(`expected dark overlay panel in center, got ${colorLabel(center)}`);
} else if (mode === "dismissed") {
  if (!isLight(center)) fail(`expected dismissed overlay to clear center panel, got ${colorLabel(center)}`);
} else {
  fail(`unknown mode: ${mode}`);
}

console.log(`android emulator ${mode} screenshot ok: ${path.basename(file)} ${png.width}x${png.height}`);

function argValue(name) {
  const match = process.argv.find((arg) => arg.startsWith(`${name}=`));
  return match ? match.slice(name.length + 1) : undefined;
}

function decodePng(buffer) {
  const signature = "89504e470d0a1a0a";
  if (buffer.subarray(0, 8).toString("hex") !== signature) fail("not a PNG file");

  let offset = 8;
  let width = 0;
  let height = 0;
  let bitDepth = 0;
  let colorType = 0;
  const idat = [];

  while (offset < buffer.length) {
    const length = buffer.readUInt32BE(offset);
    const type = buffer.subarray(offset + 4, offset + 8).toString("ascii");
    const data = buffer.subarray(offset + 8, offset + 8 + length);
    offset += 12 + length;

    if (type === "IHDR") {
      width = data.readUInt32BE(0);
      height = data.readUInt32BE(4);
      bitDepth = data[8];
      colorType = data[9];
    } else if (type === "IDAT") {
      idat.push(data);
    } else if (type === "IEND") {
      break;
    }
  }

  if (bitDepth !== 8) fail(`unsupported PNG bit depth: ${bitDepth}`);
  const channels = colorType === 6 ? 4 : colorType === 2 ? 3 : 0;
  if (!channels) fail(`unsupported PNG color type: ${colorType}`);

  const inflated = zlib.inflateSync(Buffer.concat(idat));
  const stride = width * channels;
  const pixels = Buffer.alloc(width * height * 4);
  let source = 0;
  let previous = Buffer.alloc(stride);

  for (let y = 0; y < height; y += 1) {
    const filter = inflated[source];
    source += 1;
    const row = Buffer.from(inflated.subarray(source, source + stride));
    source += stride;
    unfilter(row, previous, channels, filter);
    for (let x = 0; x < width; x += 1) {
      const src = x * channels;
      const dst = (y * width + x) * 4;
      pixels[dst] = row[src];
      pixels[dst + 1] = row[src + 1];
      pixels[dst + 2] = row[src + 2];
      pixels[dst + 3] = channels === 4 ? row[src + 3] : 255;
    }
    previous = row;
  }

  return { width, height, pixels };
}

function unfilter(row, previous, channels, filter) {
  for (let i = 0; i < row.length; i += 1) {
    const left = i >= channels ? row[i - channels] : 0;
    const up = previous[i] ?? 0;
    const upLeft = i >= channels ? previous[i - channels] : 0;
    if (filter === 1) row[i] = (row[i] + left) & 255;
    else if (filter === 2) row[i] = (row[i] + up) & 255;
    else if (filter === 3) row[i] = (row[i] + Math.floor((left + up) / 2)) & 255;
    else if (filter === 4) row[i] = (row[i] + paeth(left, up, upLeft)) & 255;
    else if (filter !== 0) fail(`unsupported PNG filter: ${filter}`);
  }
}

function paeth(a, b, c) {
  const p = a + b - c;
  const pa = Math.abs(p - a);
  const pb = Math.abs(p - b);
  const pc = Math.abs(p - c);
  if (pa <= pb && pa <= pc) return a;
  if (pb <= pc) return b;
  return c;
}

function averageRegion(png, x1, y1, x2, y2) {
  const left = Math.floor(png.width * x1);
  const top = Math.floor(png.height * y1);
  const right = Math.floor(png.width * x2);
  const bottom = Math.floor(png.height * y2);
  let r = 0;
  let g = 0;
  let b = 0;
  let count = 0;
  for (let y = top; y < bottom; y += 1) {
    for (let x = left; x < right; x += 1) {
      const offset = (y * png.width + x) * 4;
      r += png.pixels[offset];
      g += png.pixels[offset + 1];
      b += png.pixels[offset + 2];
      count += 1;
    }
  }
  return { r: r / count, g: g / count, b: b / count };
}

function isDark(color) {
  return color.r < 80 && color.g < 90 && color.b < 100;
}

function isLight(color) {
  return color.r > 220 && color.g > 220 && color.b > 220;
}

function colorLabel(color) {
  return `rgb(${Math.round(color.r)}, ${Math.round(color.g)}, ${Math.round(color.b)})`;
}

function fail(message) {
  console.error(message);
  process.exit(1);
}
