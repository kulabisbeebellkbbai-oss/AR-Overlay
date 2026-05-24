package com.kulabisbeebellkbbai.aroverlay;

final class OverlayElement {
    final String id;
    final String type;
    final float x;
    final float y;
    final float width;
    final float height;
    final float z;
    final String text;
    final int fill;
    final float opacity;
    final float fontSize;

    OverlayElement(String id, String type, float x, float y, float width, float height,
                   float z, String text, int fill, float opacity, float fontSize) {
        this.id = id;
        this.type = type;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.z = z;
        this.text = text;
        this.fill = fill;
        this.opacity = opacity;
        this.fontSize = fontSize;
    }
}
