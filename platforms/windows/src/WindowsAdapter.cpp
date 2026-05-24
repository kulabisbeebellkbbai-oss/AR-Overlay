#include "WindowsAdapter.hpp"

#include <sstream>

void WindowsAdapter::createDisplaySession() {
    lifecycle_.push_back("createDisplaySession");
}

void WindowsAdapter::presentScene(const std::string& sceneId) {
    lifecycle_.push_back("presentScene:" + sceneId);
}

void WindowsAdapter::translateInput(const std::string& action) {
    lifecycle_.push_back("translateInput:" + action);
}

void WindowsAdapter::shutdown() {
    lifecycle_.push_back("shutdown");
}

std::string WindowsAdapter::diagnosticsJson() const {
    std::ostringstream json;
    json << "{\"platform\":\"windows\",\"mode\":\"no-hardware\",\"lifecycle\":[";
    for (std::size_t index = 0; index < lifecycle_.size(); ++index) {
        if (index > 0) json << ",";
        json << "\"" << lifecycle_[index] << "\"";
    }
    json << "]}";
    return json.str();
}
