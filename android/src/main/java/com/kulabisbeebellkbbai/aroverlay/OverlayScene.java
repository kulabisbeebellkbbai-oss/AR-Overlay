package com.kulabisbeebellkbbai.aroverlay;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

final class OverlayScene {
    final String id;
    final float width;
    final float height;
    final List<OverlayElement> elements = new ArrayList<>();

    OverlayScene(String id, float width, float height) {
        this.id = id;
        this.width = width;
        this.height = height;
    }

    void addElement(OverlayElement element) {
        elements.add(element);
        elements.sort(Comparator.comparingDouble(item -> item.z));
    }

    static OverlayScene noHardwareFixture() {
        OverlayScene scene = new OverlayScene("android-no-hardware", 960, 540);
        scene.addElement(new OverlayElement("background", "rect", 160, 110, 640, 280,
                0, "", 0xff101820, 0.82f, 0));
        scene.addElement(new OverlayElement("headline", "text", 220, 178, 520, 80,
                10, "AR Overlay Android", 0xffffffff, 1f, 44));
        scene.addElement(new OverlayElement("status", "text", 220, 276, 520, 52,
                20, "No hardware mode", 0xffffcc00, 1f, 26));
        return scene;
    }
}
