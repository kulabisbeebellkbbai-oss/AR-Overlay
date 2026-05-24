package com.kulabisbeebellkbbai.aroverlay;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

final class OverlaySimulatorAdapter {
    private String state = "idle";
    private OverlayScene scene;

    JSONObject capabilities() throws JSONException {
        JSONObject display = new JSONObject()
                .put("mode", "simulated")
                .put("apiLevel", 35)
                .put("windowMode", "native-preview");
        return new JSONObject()
                .put("platform", "android")
                .put("display", display)
                .put("input", new JSONArray(OverlayContract.INPUT_ACTIONS))
                .put("permissions", new JSONObject().put("hardwareDisplay", "notRequired"))
                .put("sensors", new JSONArray())
                .put("hardwareId", JSONObject.NULL)
                .put("unsupported", new JSONArray()
                        .put(new JSONObject()
                                .put("feature", "physicalDisplay")
                                .put("reason", "no_hardware_mode")));
    }

    OverlayScene createSessionAndLoadFixture() {
        state = "sceneLoaded";
        scene = OverlayScene.noHardwareFixture();
        return scene;
    }

    String acceptInput(String action) {
        if ("dismiss".equals(action) || "back".equals(action)) {
            state = "ready";
            scene = null;
        }
        return state;
    }

    String state() {
        return state;
    }
}
