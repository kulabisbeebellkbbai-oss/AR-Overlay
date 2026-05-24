#ifdef _WIN32
#define UNICODE
#define NOMINMAX
#include <windows.h>
#include <shellscalingapi.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

struct MonitorInfo {
    HMONITOR handle;
    RECT rect;
    bool primary;
    std::wstring device;
    std::wstring label;
};

static std::vector<MonitorInfo> monitors;
static std::wstring targetName = L"XREAL";
static int durationSeconds = 20;

BOOL CALLBACK collectMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM) {
    MONITORINFOEX info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfo(monitor, &info)) return TRUE;

    DISPLAY_DEVICE device{};
    device.cb = sizeof(device);
    std::wstring label;
    if (EnumDisplayDevices(info.szDevice, 0, &device, 0)) {
        label = device.DeviceString;
    }

    monitors.push_back({
        monitor,
        info.rcMonitor,
        (info.dwFlags & MONITORINFOF_PRIMARY) != 0,
        info.szDevice,
        label
    });
    return TRUE;
}

std::wstring lower(std::wstring value) {
    for (auto& ch : value) ch = static_cast<wchar_t>(towlower(ch));
    return value;
}

MonitorInfo chooseTargetMonitor() {
    EnumDisplayMonitors(nullptr, nullptr, collectMonitor, 0);
    if (monitors.empty()) {
        throw std::runtime_error("No monitors found");
    }

    const auto wanted = lower(targetName);
    for (const auto& monitor : monitors) {
        if (lower(monitor.label).find(wanted) != std::wstring::npos ||
            lower(monitor.device).find(wanted) != std::wstring::npos) {
            return monitor;
        }
    }

    for (const auto& monitor : monitors) {
        if (!monitor.primary) return monitor;
    }

    return monitors.front();
}

void paintOverlay(HWND window) {
    PAINTSTRUCT paint{};
    HDC dc = BeginPaint(window, &paint);
    RECT client{};
    GetClientRect(window, &client);

    HBRUSH background = CreateSolidBrush(RGB(16, 24, 32));
    FillRect(dc, &client, background);
    DeleteObject(background);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));

    HFONT titleFont = CreateFontW(64, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT bodyFont = CreateFontW(34, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    RECT title{0, client.bottom / 2 - 90, client.right, client.bottom / 2 - 10};
    SelectObject(dc, titleFont);
    DrawTextW(dc, L"AR Overlay", -1, &title, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT body{0, client.bottom / 2 + 4, client.right, client.bottom / 2 + 64};
    SetTextColor(dc, RGB(255, 204, 0));
    SelectObject(dc, bodyFont);
    DrawTextW(dc, L"XREAL 1S Windows preview", -1, &body, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(titleFont);
    DeleteObject(bodyFont);
    EndPaint(window, &paint);
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_PAINT:
            paintOverlay(window);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(window);
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

std::string narrow(const std::wstring& value) {
    if (value.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string output(static_cast<std::size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, output.data(), size, nullptr, nullptr);
    return output;
}

int wmain(int argc, wchar_t** argv) {
    for (int index = 1; index < argc; ++index) {
        std::wstring arg = argv[index];
        if (arg.rfind(L"--target=", 0) == 0) targetName = arg.substr(9);
        if (arg.rfind(L"--duration=", 0) == 0) durationSeconds = std::stoi(arg.substr(11));
    }

    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    MonitorInfo target = chooseTargetMonitor();
    int width = target.rect.right - target.rect.left;
    int height = target.rect.bottom - target.rect.top;

    WNDCLASSW wc{};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"AROverlayPreviewWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HWND window = CreateWindowExW(
        WS_EX_TOPMOST,
        wc.lpszClassName,
        L"AR Overlay XREAL Preview",
        WS_POPUP,
        target.rect.left,
        target.rect.top,
        width,
        height,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (!window) {
        std::cerr << "failed to create preview window\n";
        return 1;
    }

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    std::cout
        << "{"
        << "\"platform\":\"windows\","
        << "\"mode\":\"xreal-preview\","
        << "\"target\":\"" << narrow(targetName) << "\","
        << "\"monitorDevice\":\"" << narrow(target.device) << "\","
        << "\"monitorLabel\":\"" << narrow(target.label) << "\","
        << "\"primary\":" << (target.primary ? "true" : "false") << ","
        << "\"x\":" << target.rect.left << ","
        << "\"y\":" << target.rect.top << ","
        << "\"width\":" << width << ","
        << "\"height\":" << height << ","
        << "\"durationSeconds\":" << durationSeconds
        << "}" << std::endl;

    const auto stopAt = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds);
    MSG msg{};
    while (std::chrono::steady_clock::now() < stopAt) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    DestroyWindow(window);
    return 0;
}
#else
#include <iostream>

int main() {
    std::cout
        << "{"
        << "\"platform\":\"windows\","
        << "\"mode\":\"xreal-preview\","
        << "\"status\":\"not-available-on-this-host\","
        << "\"reason\":\"Win32 preview requires Windows 11\""
        << "}" << std::endl;
    return 0;
}
#endif
