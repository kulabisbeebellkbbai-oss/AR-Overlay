import { errorCodes } from "../api/messages.js";
import { normalizeScene, validateElement } from "./scene.js";

export const sessionStates = {
  IDLE: "idle",
  READY: "ready",
  SCENE_LOADED: "sceneLoaded",
  CLOSED: "closed",
  ERROR: "error"
};

export class OverlaySession {
  constructor(adapter) {
    this.adapter = adapter;
    this.state = sessionStates.IDLE;
    this.scene = null;
    this.lastError = null;
    this.history = [];
  }

  createSession() {
    this.adapter.createDisplaySession();
    this.state = sessionStates.READY;
    this.history.push("createSession");
    return this.response(true);
  }

  loadScene(scene) {
    this.requireReady();
    this.scene = normalizeScene(scene);
    this.adapter.presentScene(this.scene);
    this.state = sessionStates.SCENE_LOADED;
    this.history.push("loadScene");
    return this.response(true, { sceneId: this.scene.id });
  }

  updateElement(element) {
    this.requireScene();
    const errors = validateElement(element);
    if (errors.length) {
      return this.fail(errorCodes.INVALID_SCENE, "Invalid element", { errors });
    }
    const next = this.scene.elements.filter((existing) => existing.id !== element.id);
    next.push(element);
    this.scene = normalizeScene({ ...this.scene, elements: next });
    this.adapter.updateScene(this.scene);
    this.history.push("updateElement");
    return this.response(true, { elementId: element.id });
  }

  removeElement(elementId) {
    this.requireScene();
    const next = this.scene.elements.filter((element) => element.id !== elementId);
    if (next.length === this.scene.elements.length) {
      return this.fail(errorCodes.ELEMENT_NOT_FOUND, "Element not found", { elementId });
    }
    this.scene = { ...this.scene, elements: next };
    this.adapter.updateScene(this.scene);
    this.history.push("removeElement");
    return this.response(true, { elementId });
  }

  acceptInput(action) {
    this.requireScene();
    this.history.push(`input:${action.type}`);
    this.adapter.translateInput(action);
    if (action.type === "dismiss" || action.type === "back") {
      this.scene = null;
      this.state = sessionStates.READY;
    }
    return this.response(true, { action: action.type });
  }

  closeSession() {
    this.scene = null;
    this.adapter.shutdown();
    this.state = sessionStates.CLOSED;
    this.history.push("closeSession");
    return this.response(true);
  }

  capabilityReport() {
    return this.adapter.capabilityReport();
  }

  response(ok, extra = {}) {
    return { ok, state: this.state, ...extra };
  }

  requireReady() {
    if (this.state !== sessionStates.READY && this.state !== sessionStates.SCENE_LOADED) {
      throw new Error(`${errorCodes.SESSION_NOT_READY}: session is not ready`);
    }
  }

  requireScene() {
    if (this.state !== sessionStates.SCENE_LOADED || !this.scene) {
      throw new Error(`${errorCodes.SESSION_NOT_READY}: scene is not loaded`);
    }
  }

  fail(code, message, detail = {}) {
    this.lastError = { code, message, detail };
    this.state = sessionStates.ERROR;
    return { ok: false, state: this.state, error: this.lastError };
  }
}
