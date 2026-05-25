#ifdef _WIN32
#define UNICODE
#define NOMINMAX
#include <windows.h>
#include <shellscalingapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using Microsoft::WRL::ComPtr;

struct MonitorInfo {
    HMONITOR handle;
    RECT rect;
    bool primary;
    std::wstring device;
    std::wstring label;
};

struct TargetChoice {
    MonitorInfo monitor;
    bool matched;
};

static std::vector<MonitorInfo> monitors;
static std::wstring targetName = L"XREAL";
static std::wstring targetDevice;
static bool allowFallback = false;
static bool requireTarget = false;
static bool listOnly = false;
static int durationSeconds = 20;
static bool running = true;

std::string hresultHex(HRESULT value) {
    std::ostringstream output;
    output << "0x" << std::hex << std::uppercase << static_cast<unsigned long>(value);
    return output.str();
}

std::string swapEffectName(DXGI_SWAP_EFFECT value) {
    switch (value) {
        case DXGI_SWAP_EFFECT_DISCARD: return "discard";
        case DXGI_SWAP_EFFECT_SEQUENTIAL: return "sequential";
        case DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL: return "flip-sequential";
        case DXGI_SWAP_EFFECT_FLIP_DISCARD: return "flip-discard";
        default: return "unknown";
    }
}

std::string driverTypeName(D3D_DRIVER_TYPE value) {
    switch (value) {
        case D3D_DRIVER_TYPE_HARDWARE: return "hardware";
        case D3D_DRIVER_TYPE_WARP: return "warp";
        case D3D_DRIVER_TYPE_REFERENCE: return "reference";
        default: return "unknown";
    }
}

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

BOOL CALLBACK collectMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM) {
    MONITORINFOEX info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) return TRUE;

    DISPLAY_DEVICEW device{};
    device.cb = sizeof(device);
    std::wstring label;
    if (EnumDisplayDevicesW(info.szDevice, 0, &device, 0)) {
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

TargetChoice chooseTargetMonitor() {
    EnumDisplayMonitors(nullptr, nullptr, collectMonitor, 0);
    if (monitors.empty()) {
        throw std::runtime_error("No monitors found");
    }

    if (!targetDevice.empty()) {
        const auto wantedDevice = lower(targetDevice);
        for (const auto& monitor : monitors) {
            if (lower(monitor.device) == wantedDevice) {
                return {monitor, true};
            }
        }

        DEVMODEW mode{};
        mode.dmSize = sizeof(mode);
        if (EnumDisplaySettingsExW(targetDevice.c_str(), ENUM_CURRENT_SETTINGS, &mode, 0)) {
            DISPLAY_DEVICEW device{};
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

    const auto wanted = lower(targetName);
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

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                running = false;
                DestroyWindow(window);
                return 0;
            }
            break;
        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int wmain(int argc, wchar_t** argv) {
    for (int index = 1; index < argc; ++index) {
        std::wstring arg = argv[index];
        if (arg.rfind(L"--target=", 0) == 0) targetName = arg.substr(9);
        if (arg.rfind(L"--device=", 0) == 0) targetDevice = arg.substr(9);
        if (arg.rfind(L"--display-number=", 0) == 0) targetDevice = L"\\\\.\\DISPLAY" + arg.substr(17);
        if (arg.rfind(L"--duration=", 0) == 0) durationSeconds = std::stoi(arg.substr(11));
        if (arg == L"--allow-fallback") allowFallback = true;
        if (arg == L"--require-target") requireTarget = true;
        if (arg == L"--list") listOnly = true;
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
    const int width = target.rect.right - target.rect.left;
    const int height = target.rect.bottom - target.rect.top;

    WNDCLASSW wc{};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"AROverlayDxgiPreviewWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HWND window = CreateWindowExW(
        WS_EX_TOPMOST,
        wc.lpszClassName,
        L"AR Overlay XREAL DXGI Preview",
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
        std::cerr << "failed to create DXGI preview window\n";
        return 1;
    }

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    D3D_FEATURE_LEVEL featureLevel{};
    DXGI_SWAP_EFFECT selectedSwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_DRIVER_TYPE selectedDriverType = D3D_DRIVER_TYPE_UNKNOWN;
    const D3D_FEATURE_LEVEL requestedLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = E_FAIL;
    const D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP
    };
    const DXGI_SWAP_EFFECT swapEffects[] = {
        DXGI_SWAP_EFFECT_FLIP_DISCARD,
        DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
        DXGI_SWAP_EFFECT_DISCARD
    };

    for (D3D_DRIVER_TYPE driverType : driverTypes) {
        for (DXGI_SWAP_EFFECT swapEffect : swapEffects) {
            DXGI_SWAP_CHAIN_DESC swapDesc{};
            swapDesc.BufferCount = (swapEffect == DXGI_SWAP_EFFECT_DISCARD) ? 1 : 2;
            swapDesc.BufferDesc.Width = static_cast<UINT>(width);
            swapDesc.BufferDesc.Height = static_cast<UINT>(height);
            swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapDesc.OutputWindow = window;
            swapDesc.SampleDesc.Count = 1;
            swapDesc.Windowed = TRUE;
            swapDesc.SwapEffect = swapEffect;

            std::cout
                << "{"
                << "\"platform\":\"windows\","
                << "\"mode\":\"xreal-dxgi-preview\","
                << "\"event\":\"create-device-attempt\","
                << "\"driverType\":\"" << driverTypeName(driverType) << "\","
                << "\"swapEffect\":\"" << swapEffectName(swapEffect) << "\""
                << "}" << std::endl;

            device.Reset();
            context.Reset();
            swapChain.Reset();
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr,
                driverType,
                nullptr,
                0,
                requestedLevels,
                static_cast<UINT>(std::size(requestedLevels)),
                D3D11_SDK_VERSION,
                &swapDesc,
                &swapChain,
                &device,
                &featureLevel,
                &context);

            if (SUCCEEDED(hr)) {
                selectedDriverType = driverType;
                selectedSwapEffect = swapEffect;
                break;
            }

            std::cerr
                << "D3D11CreateDeviceAndSwapChain failed with driver="
                << driverTypeName(driverType)
                << " swapEffect="
                << swapEffectName(swapEffect)
                << " hr="
                << hresultHex(hr)
                << "\n";
        }
        if (SUCCEEDED(hr)) break;
    }

    if (FAILED(hr)) {
        std::cerr << "D3D11CreateDeviceAndSwapChain failed after all fallback attempts: " << hresultHex(hr) << "\n";
        DestroyWindow(window);
        return 3;
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        std::cerr << "GetBuffer failed: " << hresultHex(hr) << "\n";
        DestroyWindow(window);
        return 4;
    }

    ComPtr<ID3D11RenderTargetView> renderTarget;
    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTarget);
    if (FAILED(hr)) {
        std::cerr << "CreateRenderTargetView failed: " << hresultHex(hr) << "\n";
        DestroyWindow(window);
        return 5;
    }

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    std::cout
        << "{"
        << "\"platform\":\"windows\","
        << "\"mode\":\"xreal-dxgi-preview\","
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
        << "\"driverType\":\"" << driverTypeName(selectedDriverType) << "\","
        << "\"swapEffect\":\"" << swapEffectName(selectedSwapEffect) << "\","
        << "\"featureLevel\":\"0x" << std::hex << featureLevel << std::dec << "\","
        << "\"durationSeconds\":" << durationSeconds
        << "}" << std::endl;

    const auto startedAt = std::chrono::steady_clock::now();
    const auto stopAt = startedAt + std::chrono::seconds(durationSeconds);
    int frameCount = 0;
    MSG msg{};
    while (running && std::chrono::steady_clock::now() < stopAt) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        const float phase = static_cast<float>((frameCount % 240) / 240.0);
        const float color[] = {
            0.04f + 0.12f * phase,
            0.08f,
            0.12f + 0.18f * (1.0f - phase),
            1.0f
        };
        context->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);
        context->ClearRenderTargetView(renderTarget.Get(), color);
        swapChain->Present(1, 0);
        ++frameCount;
    }

    const auto endedAt = std::chrono::steady_clock::now();
    std::cout
        << "{"
        << "\"platform\":\"windows\","
        << "\"mode\":\"xreal-dxgi-preview\","
        << "\"event\":\"summary\","
        << "\"frameCount\":" << frameCount << ","
        << "\"elapsedMs\":" << std::chrono::duration<double, std::milli>(endedAt - startedAt).count()
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
        << "\"mode\":\"xreal-dxgi-preview\","
        << "\"status\":\"not-available-on-this-host\","
        << "\"reason\":\"DirectX/DXGI preview requires Windows 11\""
        << "}" << std::endl;
    return 0;
}
#endif
