#include <iostream>

int main() {
    std::cout
        << "{\n"
        << "  \"platform\": \"linux\",\n"
        << "  \"mode\": \"no-hardware\",\n"
        << "  \"display\": {\"mode\": \"simulated\", \"compositor\": \"wayland-x11-neutral\"},\n"
        << "  \"lifecycle\": [\"createDisplaySession\", \"presentScene\", \"updateScene\", \"translateInput\", \"shutdown\"],\n"
        << "  \"hardwareRequiredNext\": [\"displayBridge\", \"presentationTiming\", \"physicalInput\", \"sensors\"]\n"
        << "}\n";
    return 0;
}
