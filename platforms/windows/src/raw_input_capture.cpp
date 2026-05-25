#ifdef _WIN32
#define UNICODE
#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

struct RawDevice {
    HANDLE handle;
    std::wstring name;
    RID_DEVICE_INFO info;
    bool xreal;
};

static std::vector<RawDevice> devices;
static std::wstring targetVid = L"VID_3318";
static std::wstring targetPid = L"PID_043E";
static int durationSeconds = 20;
static int totalEvents = 0;
static int xrealEvents = 0;

std::string narrow(const std::wstring& value) {
    if (value.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string output(static_cast<std::size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, output.data(), size, nullptr, nullptr);
    return output;
}

std::wstring lower(std::wstring value) {
    for (auto& ch : value) ch = static_cast<wchar_t>(towlower(ch));
    return value;
}

std::string jsonEscape(const std::string& value) {
    std::string output;
    output.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
            case '\\': output += "\\\\"; break;
            case '"': output += "\\\""; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += ch; break;
        }
    }
    return output;
}

std::string hexBytes(const BYTE* bytes, DWORD size) {
    std::ostringstream out;
    const DWORD limit = std::min<DWORD>(size, 32);
    for (DWORD index = 0; index < limit; ++index) {
        if (index > 0) out << " ";
        out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[index]);
    }
    return out.str();
}

std::string typeName(DWORD type) {
    switch (type) {
        case RIM_TYPEMOUSE: return "mouse";
        case RIM_TYPEKEYBOARD: return "keyboard";
        case RIM_TYPEHID: return "hid";
        default: return "unknown";
    }
}

bool isXrealName(const std::wstring& name) {
    const auto lowered = lower(name);
    return lowered.find(lower(targetVid)) != std::wstring::npos ||
        lowered.find(lower(targetPid)) != std::wstring::npos ||
        lowered.find(L"xreal") != std::wstring::npos ||
        lowered.find(L"nreal") != std::wstring::npos;
}

RawDevice describeDevice(HANDLE handle) {
    UINT nameSize = 0;
    GetRawInputDeviceInfoW(handle, RIDI_DEVICENAME, nullptr, &nameSize);
    std::wstring name(nameSize, L'\0');
    if (nameSize > 0) {
        GetRawInputDeviceInfoW(handle, RIDI_DEVICENAME, name.data(), &nameSize);
        if (!name.empty() && name.back() == L'\0') name.pop_back();
    }

    RID_DEVICE_INFO info{};
    info.cbSize = sizeof(info);
    UINT infoSize = sizeof(info);
    GetRawInputDeviceInfoW(handle, RIDI_DEVICEINFO, &info, &infoSize);
    return RawDevice{handle, name, info, isXrealName(name)};
}

void enumerateDevices() {
    UINT count = 0;
    GetRawInputDeviceList(nullptr, &count, sizeof(RAWINPUTDEVICELIST));
    std::vector<RAWINPUTDEVICELIST> list(count);
    if (count > 0) {
        GetRawInputDeviceList(list.data(), &count, sizeof(RAWINPUTDEVICELIST));
    }

    devices.clear();
    for (const auto& item : list) {
        devices.push_back(describeDevice(item.hDevice));
    }
}

const RawDevice* findDevice(HANDLE handle) {
    for (const auto& device : devices) {
        if (device.handle == handle) return &device;
    }
    return nullptr;
}

void printDeviceList() {
    std::cout << "{\"platform\":\"windows\",\"mode\":\"xreal-raw-input-capture\",\"event\":\"device-list\",\"devices\":[";
    for (std::size_t index = 0; index < devices.size(); ++index) {
        const auto& device = devices[index];
        if (index > 0) std::cout << ",";
        std::cout
            << "{"
            << "\"name\":\"" << jsonEscape(narrow(device.name)) << "\","
            << "\"type\":\"" << typeName(device.info.dwType) << "\","
            << "\"xreal\":" << (device.xreal ? "true" : "false");
        if (device.info.dwType == RIM_TYPEHID) {
            std::cout
                << ",\"vendorId\":" << device.info.hid.dwVendorId
                << ",\"productId\":" << device.info.hid.dwProductId
                << ",\"usagePage\":" << device.info.hid.usUsagePage
                << ",\"usage\":" << device.info.hid.usUsage;
        }
        std::cout << "}";
    }
    std::cout << "]}" << std::endl;
}

void printRawInput(HRAWINPUT inputHandle) {
    UINT size = 0;
    GetRawInputData(inputHandle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
    std::vector<BYTE> buffer(size);
    if (GetRawInputData(inputHandle, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1)) {
        return;
    }

    const RAWINPUT* input = reinterpret_cast<const RAWINPUT*>(buffer.data());
    const RawDevice* device = findDevice(input->header.hDevice);
    const bool xreal = device != nullptr && device->xreal;
    ++totalEvents;
    if (xreal) ++xrealEvents;

    std::cout
        << "{\"platform\":\"windows\",\"mode\":\"xreal-raw-input-capture\",\"event\":\"raw-input\","
        << "\"type\":\"" << typeName(input->header.dwType) << "\","
        << "\"xreal\":" << (xreal ? "true" : "false") << ","
        << "\"device\":\"" << jsonEscape(device ? narrow(device->name) : "") << "\"";

    if (input->header.dwType == RIM_TYPEKEYBOARD) {
        std::cout
            << ",\"vkey\":" << input->data.keyboard.VKey
            << ",\"message\":" << input->data.keyboard.Message
            << ",\"flags\":" << input->data.keyboard.Flags;
    } else if (input->header.dwType == RIM_TYPEMOUSE) {
        std::cout
            << ",\"buttonFlags\":" << input->data.mouse.usButtonFlags
            << ",\"buttonData\":" << input->data.mouse.usButtonData
            << ",\"lastX\":" << input->data.mouse.lLastX
            << ",\"lastY\":" << input->data.mouse.lLastY;
    } else if (input->header.dwType == RIM_TYPEHID) {
        const DWORD reportBytes = input->data.hid.dwSizeHid * input->data.hid.dwCount;
        std::cout
            << ",\"hidSize\":" << input->data.hid.dwSizeHid
            << ",\"hidCount\":" << input->data.hid.dwCount
            << ",\"hidBytes\":\"" << hexBytes(input->data.hid.bRawData, reportBytes) << "\"";
    }

    std::cout << "}" << std::endl;
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INPUT:
            printRawInput(reinterpret_cast<HRAWINPUT>(lParam));
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

bool registerRawInput(HWND window) {
    RAWINPUTDEVICE registrations[] = {
        {0x01, 0x02, RIDEV_INPUTSINK, window},
        {0x01, 0x06, RIDEV_INPUTSINK, window},
        {0x01, 0x80, RIDEV_INPUTSINK, window},
        {0x0c, 0x01, RIDEV_INPUTSINK, window},
        {0xff00, 0x00, RIDEV_INPUTSINK | RIDEV_PAGEONLY, window},
        {0xff01, 0x00, RIDEV_INPUTSINK | RIDEV_PAGEONLY, window}
    };
    return RegisterRawInputDevices(
        registrations,
        static_cast<UINT>(std::size(registrations)),
        sizeof(RAWINPUTDEVICE)) == TRUE;
}

int wmain(int argc, wchar_t** argv) {
    for (int index = 1; index < argc; ++index) {
        std::wstring arg = argv[index];
        if (arg.rfind(L"--duration=", 0) == 0) durationSeconds = std::stoi(arg.substr(11));
        if (arg.rfind(L"--target-vid=", 0) == 0) targetVid = arg.substr(13);
        if (arg.rfind(L"--target-pid=", 0) == 0) targetPid = arg.substr(13);
    }

    enumerateDevices();
    printDeviceList();

    WNDCLASSW wc{};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"AROverlayRawInputCaptureWindow";
    RegisterClassW(&wc);

    HWND window = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"AR Overlay Raw Input Capture",
        0,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        nullptr,
        wc.hInstance,
        nullptr);

    if (!window) {
        std::cerr << "failed to create raw input message window\n";
        return 1;
    }

    if (!registerRawInput(window)) {
        std::cerr << "failed to register raw input devices: " << GetLastError() << "\n";
        DestroyWindow(window);
        return 2;
    }

    std::cout
        << "{\"platform\":\"windows\",\"mode\":\"xreal-raw-input-capture\",\"event\":\"capture-start\","
        << "\"durationSeconds\":" << durationSeconds
        << ",\"targetVid\":\"" << jsonEscape(narrow(targetVid)) << "\""
        << ",\"targetPid\":\"" << jsonEscape(narrow(targetPid)) << "\""
        << "}" << std::endl;

    const auto stopAt = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds);
    MSG msg{};
    while (std::chrono::steady_clock::now() < stopAt) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout
        << "{\"platform\":\"windows\",\"mode\":\"xreal-raw-input-capture\",\"event\":\"summary\","
        << "\"totalEvents\":" << totalEvents
        << ",\"xrealEvents\":" << xrealEvents
        << "}" << std::endl;

    DestroyWindow(window);
    return 0;
}
#else
#include <iostream>

int main() {
    std::cout
        << "{"
        << "\"platform\":\"windows\","
        << "\"mode\":\"xreal-raw-input-capture\","
        << "\"status\":\"not-available-on-this-host\","
        << "\"reason\":\"Win32 Raw Input capture requires Windows 11\""
        << "}" << std::endl;
    return 0;
}
#endif
