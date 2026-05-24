#pragma once

#include <string>
#include <vector>

class WindowsAdapter {
public:
    void createDisplaySession();
    void presentScene(const std::string& sceneId);
    void translateInput(const std::string& action);
    void shutdown();
    std::string diagnosticsJson() const;

private:
    std::vector<std::string> lifecycle_;
};
