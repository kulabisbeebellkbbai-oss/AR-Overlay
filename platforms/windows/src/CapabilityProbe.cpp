#include "CapabilityProbe.hpp"

std::string windowsCapabilityJson() {
    return R"json({
  "platform": "windows",
  "display": {"mode": "native-preview", "osFamily": "windows-11"},
  "input": ["select", "back", "dismiss", "drag", "pinch", "scroll", "text"],
  "permissions": {"hardwareDisplay": "notRequired"},
  "sensors": [],
  "hardwareId": null,
  "unsupported": [{"feature": "physicalDisplay", "reason": "no_hardware_mode"}]
})json";
}
