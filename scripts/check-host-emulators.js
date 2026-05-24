import { existsSync } from "node:fs";
import { spawnSync } from "node:child_process";

const checks = [
  {
    name: "linux",
    emulator: "repo-native",
    status: "available",
    detail: "Node.js no-hardware emulator runs on the local Linux host."
  },
  androidCheck(),
  iosCheck(),
  windowsCheck()
];

for (const check of checks) {
  console.log(`${check.name}: ${check.status} (${check.emulator}) - ${check.detail}`);
}

const blocking = checks.filter((check) => check.status === "blocked");
if (process.argv.includes("--strict") && blocking.length) {
  process.exitCode = 1;
}

function androidCheck() {
  const emulator = commandExists("emulator");
  const adb = commandExists("adb");
  const waydroid = commandExists("waydroid");
  const hasKvm = existsSync("/dev/kvm");
  const waydroidStatus = waydroid ? commandOutput("waydroid", ["status"]) : "";
  const waydroidBooted = waydroidStatus.includes("Session:\tRUNNING");

  if (emulator && adb && hasKvm) {
    return {
      name: "android",
      emulator: "Android SDK Emulator",
      status: "available",
      detail: "Android SDK emulator tooling is available with KVM acceleration."
    };
  }

  if (waydroid && waydroidBooted) {
    return {
      name: "android",
      emulator: "Waydroid",
      status: "available-limited",
      detail: "Waydroid is running; Android SDK AVD acceleration is blocked because /dev/kvm is missing."
    };
  }

  return {
    name: "android",
    emulator: "Android SDK Emulator",
    status: "blocked",
    detail: emulator && adb ? "Android tooling exists, but /dev/kvm is missing and Waydroid is not running." : "Android emulator tooling is missing."
  };
}

function iosCheck() {
  if (commandExists("xcrun")) {
    return {
      name: "ios",
      emulator: "Apple Simulator",
      status: "available",
      detail: "xcrun is available."
    };
  }
  return {
    name: "ios",
    emulator: "Apple Simulator",
    status: "blocked",
    detail: "Apple iOS Simulator requires macOS/Xcode and is not available on this Debian host."
  };
}

function windowsCheck() {
  const qemu = commandExists("qemu-system-x86_64");
  const wine = commandExists("wine");
  if (qemu || wine) {
    return {
      name: "windows",
      emulator: qemu ? "QEMU" : "Wine",
      status: "available-limited",
      detail: "A runtime tool exists, but no Windows 11 VM image or packaged Windows build is configured in this repo."
    };
  }
  return {
    name: "windows",
    emulator: "Windows VM",
    status: "blocked",
    detail: "No Windows runtime emulator is available."
  };
}

function commandExists(command) {
  return spawnSync("bash", ["-lc", `command -v ${command}`], { encoding: "utf8" }).status === 0;
}

function commandOutput(command, args) {
  return spawnSync(command, args, { encoding: "utf8" }).stdout ?? "";
}
