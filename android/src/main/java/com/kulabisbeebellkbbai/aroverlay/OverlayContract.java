package com.kulabisbeebellkbbai.aroverlay;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

final class OverlayContract {
    static final String VERSION = "0.1";
    static final Set<String> INPUT_ACTIONS = new HashSet<>(
            Arrays.asList("select", "back", "dismiss", "drag", "pinch", "scroll", "text"));

    private OverlayContract() {
    }
}
