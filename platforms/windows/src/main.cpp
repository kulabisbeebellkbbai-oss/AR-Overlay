#include "CapabilityProbe.hpp"
#include "FixtureRunner.hpp"
#include "WindowsAdapter.hpp"

#include <iostream>

int main() {
    WindowsAdapter adapter;
    adapter.createDisplaySession();
    for (const auto& step : noHardwareScenario()) {
        if (step.command == "loadScene") adapter.presentScene("windows-no-hardware");
        if (step.id == "select" || step.id == "dismiss") adapter.translateInput(step.id);
    }
    adapter.shutdown();

    std::cout << windowsCapabilityJson() << "\n";
    std::cout << adapter.diagnosticsJson() << "\n";
    return 0;
}
