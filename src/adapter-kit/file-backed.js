import { mkdirSync, renameSync, writeFileSync } from "node:fs";
import { dirname } from "node:path";
import { NoHardwareAdapter } from "./index.js";

export class FileBackedAdapter extends NoHardwareAdapter {
  constructor(platform, options = {}) {
    super(platform, options.overrides);
    this.stateFile = options.stateFile;
    this.revision = 0;
    if (!this.stateFile) throw new Error("FileBackedAdapter requires stateFile");
    this.writeState({ state: "idle", scene: null });
  }

  createDisplaySession() {
    const result = super.createDisplaySession();
    this.writeState({ state: "ready", scene: null });
    return result;
  }

  presentScene(scene) {
    const result = super.presentScene(scene);
    this.writeState({ state: "sceneLoaded", scene });
    return result;
  }

  updateScene(scene) {
    const result = super.updateScene(scene);
    this.writeState({ state: "sceneLoaded", scene });
    return result;
  }

  translateInput(event) {
    const result = super.translateInput(event);
    if (event.type === "dismiss" || event.type === "back") {
      this.writeState({ state: "ready", scene: null });
    }
    return result;
  }

  shutdown() {
    const result = super.shutdown();
    this.writeState({ state: "closed", scene: null });
    return result;
  }

  writeState({ state, scene }) {
    this.revision += 1;
    const payload = {
      version: "0.1",
      platform: this.platform,
      mode: "dxgi-live-state",
      revision: this.revision,
      updatedAt: new Date().toISOString(),
      state,
      sceneId: scene?.id ?? null,
      primaryText: firstText(scene) ?? scene?.id ?? state,
      secondaryText: scene ? "Live API scene update" : "Waiting for live API scene",
      scene
    };
    mkdirSync(dirname(this.stateFile), { recursive: true });
    const tmp = `${this.stateFile}.tmp`;
    writeFileSync(tmp, `${JSON.stringify(payload, null, 2)}\n`, "utf8");
    renameSync(tmp, this.stateFile);
  }
}

function firstText(scene) {
  return scene?.elements?.find((element) => element.type === "text")?.text;
}
