package com.kulabisbeebellkbbai.aroverlay;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.view.View;

final class OverlayView extends View {
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private OverlayScene scene;

    OverlayView(Context context) {
        super(context);
        setFocusable(true);
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }

    void setScene(OverlayScene scene) {
        this.scene = scene;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (scene == null) return;

        float scale = Math.min(getWidth() / scene.width, getHeight() / scene.height);
        float offsetX = (getWidth() - scene.width * scale) / 2f;
        float offsetY = (getHeight() - scene.height * scale) / 2f;
        for (OverlayElement element : scene.elements) {
            paint.setAlpha(Math.round(element.opacity * 255));
            paint.setColor(element.fill);
            if ("rect".equals(element.type)) {
                canvas.drawRect(offsetX + element.x * scale, offsetY + element.y * scale,
                        offsetX + (element.x + element.width) * scale,
                        offsetY + (element.y + element.height) * scale, paint);
            } else if ("text".equals(element.type)) {
                paint.setTextSize(element.fontSize * scale);
                paint.setTextAlign(Paint.Align.CENTER);
                canvas.drawText(element.text,
                        offsetX + (element.x + element.width / 2f) * scale,
                        offsetY + (element.y + element.fontSize) * scale,
                        paint);
            }
        }
    }
}
