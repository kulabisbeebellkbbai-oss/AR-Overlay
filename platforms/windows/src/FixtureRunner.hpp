#pragma once

#include <string>
#include <vector>

struct FixtureStep {
    std::string id;
    std::string command;
};

std::vector<FixtureStep> noHardwareScenario();
