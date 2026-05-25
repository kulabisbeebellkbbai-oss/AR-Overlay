import { existsSync, readFileSync } from "node:fs";
import { spawnSync } from "node:child_process";
import path from "node:path";

const checks = [];

checkAndroidScaffold();
checkIosScaffold();
checkCmakeScaffold("linux", "platforms/linux", "build/platforms/linux", "ar-overlay-linux-smoke");
checkCmakeScaffold("windows", "platforms/windows", "build/platforms/windows-host-check", "ar-overlay-windows-smoke");

for (const check of checks) {
  console.log(`${check.name}: ${check.ok ? "ok" : "failed"}${check.detail ? ` - ${check.detail}` : ""}`);
}

if (checks.some((check) => !check.ok)) process.exitCode = 1;

function checkAndroidScaffold() {
  const gradle = read("android/build.gradle");
  const manifestExists = existsSync("android/src/main/AndroidManifest.xml");
  const javaExists = existsSync("android/src/main/java/com/kulabisbeebellkbbai/aroverlay/MainActivity.java");
  const ok = gradle.includes("compileSdk 36")
    && gradle.includes("targetSdk 35")
    && gradle.includes("minSdk 26")
    && manifestExists
    && javaExists;
  checks.push({
    name: "android scaffold static validation",
    ok,
    detail: "static contract ok; use npm run android:start-emulator && npm run android:emulator-smoke for runtime validation"
  });
}

function checkIosScaffold() {
  const project = read("platforms/ios/AROverlay.xcodeproj/project.pbxproj");
  const probe = read("platforms/ios/AROverlay/IOSCapabilityProbe.swift");
  const ok = project.includes("IPHONEOS_DEPLOYMENT_TARGET = 15.0")
    && probe.includes("ARWorldTrackingConfiguration.isSupported");
  checks.push({
    name: "ios scaffold static validation",
    ok,
    detail: "native build requires macOS/Xcode"
  });
}

function checkCmakeScaffold(name, sourceDir, buildDir, binaryName) {
  const configure = run("cmake", ["-S", sourceDir, "-B", buildDir]);
  if (configure.status !== 0) {
    checks.push({ name: `${name} cmake configure`, ok: false, detail: configure.stderr || configure.stdout });
    return;
  }

  const build = run("cmake", ["--build", buildDir]);
  if (build.status !== 0) {
    checks.push({ name: `${name} cmake build`, ok: false, detail: build.stderr || build.stdout });
    return;
  }

  const binaryPath = path.join(buildDir, binaryName);
  const smoke = run(binaryPath, []);
  checks.push({
    name: `${name} native smoke`,
    ok: smoke.status === 0 && smoke.stdout.includes(`"platform"`) && smoke.stdout.includes(name),
    detail: smoke.status === 0 ? "native no-hardware binary executed" : smoke.stderr
  });
}

function read(file) {
  return readFileSync(file, "utf8");
}

function run(command, args) {
  return spawnSync(command, args, { encoding: "utf8" });
}
