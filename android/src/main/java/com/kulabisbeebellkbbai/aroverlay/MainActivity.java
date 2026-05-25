package com.kulabisbeebellkbbai.aroverlay;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;

public final class MainActivity extends Activity {
    private final OverlaySimulatorAdapter adapter = new OverlaySimulatorAdapter();
    private OverlayView view;

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        view = new OverlayView(this);
        updateView(adapter.createSessionAndLoadFixture());
        setContentView(view);

        try {
            Log.i("AROverlay", adapter.capabilities().toString());
        } catch (Exception error) {
            Log.e("AROverlay", "Capability report failed", error);
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            adapter.acceptInput("back");
            updateView(null);
            return true;
        }
        return super.onKeyUp(keyCode, event);
    }

    private void updateView(OverlayScene scene) {
        view.setScene(scene);
        view.setContentDescription("ar-overlay-state:" + adapter.state());
    }
}
