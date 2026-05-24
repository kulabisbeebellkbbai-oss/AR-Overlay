import os from "node:os";

export class NoHardwareAdapter {
  constructor(platform, overrides = {}) {
    this.platform = platform;
    this.overrides = overrides;
    this.lifecycle = [];
  }

  capabilityReport() {
    return {
      platform: this.platform,
      display: {
        mode: "simulated",
        resolutionClass: "fixture",
        refreshRate: "deterministic",
        ...this.overrides.display
      },
      input: this.overrides.input ?? ["select", "back", "dismiss", "drag", "pinch", "scroll", "text"],
      permissions: {
        hardwareDisplay: "notRequired",
        camera: "notRequired",
        sensors: "simulated",
        ...this.overrides.permissions
      },
      sensors: this.overrides.sensors ?? [],
      hardwareId: null,
      host: {
        os: os.platform(),
        arch: os.arch(),
        release: os.release()
      },
      unsupported: this.overrides.unsupported ?? [
        {
          feature: "physicalDisplay",
          reason: "no_hardware_mode"
        }
      ]
    };
  }

  createDisplaySession() {
    this.lifecycle.push("createDisplaySession");
    return { ok: true, mode: "simulated" };
  }

  presentScene(scene) {
    this.lifecycle.push("presentScene");
    return { ok: true, sceneId: scene.id, frameMode: "svg" };
  }

  updateScene(scene) {
    this.lifecycle.push("updateScene");
    return { ok: true, sceneId: scene.id };
  }

  translateInput(event) {
    this.lifecycle.push("translateInput");
    return event;
  }

  recoverPermission(permission) {
    this.lifecycle.push("recoverPermission");
    return { ok: true, permission, state: "notRequired" };
  }

  diagnostics() {
    this.lifecycle.push("diagnostics");
    return {
      platform: this.platform,
      mode: "no-hardware",
      lifecycle: [...this.lifecycle]
    };
  }

  shutdown() {
    this.lifecycle.push("shutdown");
    return { ok: true };
  }
}

export function createPlatformAdapter(platform) {
  const adapterDefaults = {
    linux: {
      display: { compositor: "simulated-wayland-x11-neutral" },
      sensors: ["simulated-head-pose"]
    },
    android: {
      display: { apiLevel: 35, windowMode: "simulated-edge-to-edge" },
      permissions: { foregroundService: "simulated" }
    },
    ios: {
      display: { sdkGeneration: "ios-26", minimumDeploymentTarget: "ios-15" },
      permissions: { arkit: "simulated" }
    },
    windows: {
      display: { osFamily: "windows-11", graphicsBackend: "simulated" },
      permissions: { driverAccess: "simulated" }
    }
  };

  if (!adapterDefaults[platform]) {
    throw new Error(`Unsupported platform adapter: ${platform}`);
  }
  return new NoHardwareAdapter(platform, adapterDefaults[platform]);
}
