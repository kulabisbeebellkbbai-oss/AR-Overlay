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
    }

    void setScene(OverlayScene scene) {
        this.scene = scene;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (scene == null) return;

        float scaleX = getWidth() / scene.width;
        float scaleY = getHeight() / scene.height;
        for (OverlayElement element : scene.elements) {
            paint.setAlpha(Math.round(element.opacity * 255));
            paint.setColor(element.fill);
            if ("rect".equals(element.type)) {
                canvas.drawRect(element.x * scaleX, element.y * scaleY,
                        (element.x + element.width) * scaleX,
                        (element.y + element.height) * scaleY, paint);
            } else if ("text".equals(element.type)) {
                paint.setTextSize(element.fontSize * scaleY);
                paint.setTextAlign(Paint.Align.CENTER);
                canvas.drawText(element.text,
                        (element.x + element.width / 2f) * scaleX,
                        (element.y + element.fontSize) * scaleY,
                        paint);
            }
        }
    }
}
