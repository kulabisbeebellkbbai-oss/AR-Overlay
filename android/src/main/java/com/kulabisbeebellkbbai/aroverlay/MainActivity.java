package com.kulabisbeebellkbbai.aroverlay;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public final class MainActivity extends Activity {
    private final OverlaySimulatorAdapter adapter = new OverlaySimulatorAdapter();

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        OverlayView view = new OverlayView(this);
        view.setScene(adapter.createSessionAndLoadFixture());
        setContentView(view);

        try {
            Log.i("AROverlay", adapter.capabilities().toString());
        } catch (Exception error) {
            Log.e("AROverlay", "Capability report failed", error);
        }
    }
}
