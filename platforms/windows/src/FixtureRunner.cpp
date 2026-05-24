#include "FixtureRunner.hpp"

std::vector<FixtureStep> noHardwareScenario() {
    return {
        {"create-session", "createSession"},
        {"load-scene", "loadScene"},
        {"update-element", "updateElement"},
        {"select", "input"},
        {"dismiss", "input"},
        {"close-session", "closeSession"}
    };
}
