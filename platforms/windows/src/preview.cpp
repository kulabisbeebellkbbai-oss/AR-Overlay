#ifdef _WIN32
#define UNICODE
#define NOMINMAX
#include <windows.h>
#include <shellscalingapi.h>
#include <mmsystem.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
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
static std::wstring targetDevice;
static bool allowFallback = false;
static int durationSeconds = 20;
static bool listOnly = false;
static bool requireTarget = false;
static bool measureTiming = false;

std::string narrow(const std::wstring& value);
std::string jsonEscape(const std::string& value);
void printTimingReport(const std::vector<double>& frameIntervalsMs, double elapsedMs, int targetRefreshHz);

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

struct TargetChoice {
    MonitorInfo monitor;
    bool matched;
};

TargetChoice chooseTargetMonitor() {
    EnumDisplayMonitors(nullptr, nullptr, collectMonitor, 0);
    if (monitors.empty()) {
        throw std::runtime_error("No monitors found");
    }

    const auto wanted = lower(targetName);
    if (!targetDevice.empty()) {
        const auto wantedDevice = lower(targetDevice);
        for (const auto& monitor : monitors) {
            if (lower(monitor.device) == wantedDevice) {
                return {monitor, true};
            }
        }

        DEVMODE mode{};
        mode.dmSize = sizeof(mode);
        if (EnumDisplaySettingsExW(targetDevice.c_str(), ENUM_CURRENT_SETTINGS, &mode, 0)) {
            DISPLAY_DEVICE device{};
            device.cb = sizeof(device);
            std::wstring label = targetName;
            if (EnumDisplayDevicesW(targetDevice.c_str(), 0, &device, 0)) {
                label = device.DeviceString;
            }
            RECT rect{
                mode.dmPosition.x,
                mode.dmPosition.y,
                mode.dmPosition.x + static_cast<LONG>(mode.dmPelsWidth),
                mode.dmPosition.y + static_cast<LONG>(mode.dmPelsHeight)
            };
            return {MonitorInfo{nullptr, rect, false, targetDevice, label}, true};
        }
    }

    for (const auto& monitor : monitors) {
        if (lower(monitor.label).find(wanted) != std::wstring::npos ||
            lower(monitor.device).find(wanted) != std::wstring::npos) {
            return {monitor, true};
        }
    }

    for (const auto& monitor : monitors) {
        if (!monitor.primary) {
            const int width = monitor.rect.right - monitor.rect.left;
            const int height = monitor.rect.bottom - monitor.rect.top;
            if (width == 1920 && height == 1200) return {monitor, allowFallback};
        }
    }

    for (const auto& monitor : monitors) {
        if (!monitor.primary) return {monitor, allowFallback};
    }

    return {monitors.front(), false};
}

void printMonitorList() {
    std::cout << "{\"monitors\":[";
    for (std::size_t index = 0; index < monitors.size(); ++index) {
        const auto& monitor = monitors[index];
        if (index > 0) std::cout << ",";
        std::cout
            << "{"
            << "\"device\":\"" << jsonEscape(narrow(monitor.device)) << "\","
            << "\"label\":\"" << jsonEscape(narrow(monitor.label)) << "\","
            << "\"primary\":" << (monitor.primary ? "true" : "false") << ","
            << "\"x\":" << monitor.rect.left << ","
            << "\"y\":" << monitor.rect.top << ","
            << "\"width\":" << (monitor.rect.right - monitor.rect.left) << ","
            << "\"height\":" << (monitor.rect.bottom - monitor.rect.top)
            << "}";
    }
    std::cout << "]}" << std::endl;
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

int wmain(int argc, wchar_t** argv) {
    for (int index = 1; index < argc; ++index) {
        std::wstring arg = argv[index];
        if (arg.rfind(L"--target=", 0) == 0) targetName = arg.substr(9);
        if (arg.rfind(L"--device=", 0) == 0) targetDevice = arg.substr(9);
        if (arg.rfind(L"--display-number=", 0) == 0) targetDevice = L"\\\\.\\DISPLAY" + arg.substr(17);
        if (arg.rfind(L"--duration=", 0) == 0) durationSeconds = std::stoi(arg.substr(11));
        if (arg == L"--allow-fallback") allowFallback = true;
        if (arg == L"--list") listOnly = true;
        if (arg == L"--require-target") requireTarget = true;
        if (arg == L"--measure-timing") measureTiming = true;
    }

    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    TargetChoice choice = chooseTargetMonitor();
    printMonitorList();
    if (listOnly) return 0;
    if (requireTarget && !choice.matched) {
        std::cerr << "target monitor not found: " << narrow(targetName) << "\n";
        return 2;
    }

    MonitorInfo target = choice.monitor;
    int width = target.rect.right - target.rect.left;
    int height = target.rect.bottom - target.rect.top;
    int targetRefreshHz = 0;
    if (!target.device.empty()) {
        DEVMODE mode{};
        mode.dmSize = sizeof(mode);
        if (EnumDisplaySettingsExW(target.device.c_str(), ENUM_CURRENT_SETTINGS, &mode, 0)) {
            targetRefreshHz = static_cast<int>(mode.dmDisplayFrequency);
        }
    }

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
        << "\"target\":\"" << jsonEscape(narrow(targetName)) << "\","
        << "\"targetDevice\":\"" << jsonEscape(narrow(targetDevice)) << "\","
        << "\"targetMatched\":" << (choice.matched ? "true" : "false") << ","
        << "\"fallbackAllowed\":" << (allowFallback ? "true" : "false") << ","
        << "\"monitorDevice\":\"" << jsonEscape(narrow(target.device)) << "\","
        << "\"monitorLabel\":\"" << jsonEscape(narrow(target.label)) << "\","
        << "\"primary\":" << (target.primary ? "true" : "false") << ","
        << "\"x\":" << target.rect.left << ","
        << "\"y\":" << target.rect.top << ","
        << "\"width\":" << width << ","
        << "\"height\":" << height << ","
        << "\"refreshHz\":" << targetRefreshHz << ","
        << "\"durationSeconds\":" << durationSeconds
        << "}" << std::endl;

    const auto stopAt = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds);
    const auto startedAt = std::chrono::steady_clock::now();
    auto previousFrame = startedAt;
    auto nextFrame = startedAt;
    const double targetFrameMs = targetRefreshHz > 0 ? 1000.0 / static_cast<double>(targetRefreshHz) : 16.6667;
    const auto frameStep = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double, std::milli>(targetFrameMs));
    std::vector<double> frameIntervalsMs;
    bool highResolutionTimer = false;
    if (measureTiming && timeBeginPeriod(1) == TIMERR_NOERROR) {
        highResolutionTimer = true;
    }
    MSG msg{};
    while (std::chrono::steady_clock::now() < stopAt) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (measureTiming) {
            const auto now = std::chrono::steady_clock::now();
            frameIntervalsMs.push_back(std::chrono::duration<double, std::milli>(now - previousFrame).count());
            previousFrame = now;
            InvalidateRect(window, nullptr, FALSE);
            UpdateWindow(window);
            nextFrame += frameStep;
            std::this_thread::sleep_until(nextFrame);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    if (measureTiming) {
        const auto endedAt = std::chrono::steady_clock::now();
        if (highResolutionTimer) timeEndPeriod(1);
        printTimingReport(frameIntervalsMs, std::chrono::duration<double, std::milli>(endedAt - startedAt).count(), targetRefreshHz);
    }

    DestroyWindow(window);
    return 0;
}

void printTimingReport(const std::vector<double>& frameIntervalsMs, double elapsedMs, int targetRefreshHz) {
    double minFrameMs = 0;
    double maxFrameMs = 0;
    double avgFrameMs = 0;
    double targetFrameMs = targetRefreshHz > 0 ? 1000.0 / static_cast<double>(targetRefreshHz) : 16.6667;
    int over20Ms = 0;
    int over33Ms = 0;
    int overTargetBy2Ms = 0;
    if (!frameIntervalsMs.empty()) {
        minFrameMs = std::numeric_limits<double>::max();
        for (double value : frameIntervalsMs) {
            minFrameMs = std::min(minFrameMs, value);
            maxFrameMs = std::max(maxFrameMs, value);
            avgFrameMs += value;
            if (value > 20.0) ++over20Ms;
            if (value > 33.0) ++over33Ms;
            if (value > targetFrameMs + 2.0) ++overTargetBy2Ms;
        }
        avgFrameMs /= static_cast<double>(frameIntervalsMs.size());
    }

    std::cout
        << "{"
        << "\"platform\":\"windows\","
        << "\"mode\":\"xreal-preview-timing\","
        << "\"frameCount\":" << frameIntervalsMs.size() << ","
        << "\"elapsedMs\":" << elapsedMs << ","
        << "\"targetRefreshHz\":" << targetRefreshHz << ","
        << "\"targetFrameMs\":" << targetFrameMs << ","
        << "\"minFrameMs\":" << minFrameMs << ","
        << "\"avgFrameMs\":" << avgFrameMs << ","
        << "\"maxFrameMs\":" << maxFrameMs << ","
        << "\"framesOver20Ms\":" << over20Ms << ","
        << "\"framesOver33Ms\":" << over33Ms << ","
        << "\"framesOverTargetBy2Ms\":" << overTargetBy2Ms
        << "}" << std::endl;
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
