#ifdef _WIN32
#define UNICODE
#define NOMINMAX
#include <windows.h>
#include <shellscalingapi.h>
#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <algorithm>
#include <chrono>
#include <cwchar>
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

struct FrameStats {
    int frameCount = 0;
    int presentFailures = 0;
    int framesOver20Ms = 0;
    int framesOver33Ms = 0;
    int framesOverTargetBy2Ms = 0;
    double minFrameMs = 0;
    double avgFrameMs = 0;
    double maxFrameMs = 0;
    HRESULT lastPresentFailure = S_OK;
};

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

double targetFrameMsForRefresh(int refreshHz) {
    return refreshHz > 0 ? 1000.0 / static_cast<double>(refreshHz) : 16.6667;
}

void recordFrameInterval(FrameStats& stats, double frameMs, double targetFrameMs) {
    if (stats.frameCount == 0) {
        stats.minFrameMs = frameMs;
        stats.maxFrameMs = frameMs;
    } else {
        stats.minFrameMs = std::min(stats.minFrameMs, frameMs);
        stats.maxFrameMs = std::max(stats.maxFrameMs, frameMs);
    }
    stats.avgFrameMs += frameMs;
    if (frameMs > 20.0) ++stats.framesOver20Ms;
    if (frameMs > 33.0) ++stats.framesOver33Ms;
    if (frameMs > targetFrameMs + 2.0) ++stats.framesOverTargetBy2Ms;
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

int refreshRateForMonitor(const MonitorInfo& monitor) {
    if (monitor.device.empty()) return 0;
    DEVMODEW mode{};
    mode.dmSize = sizeof(mode);
    if (EnumDisplaySettingsExW(monitor.device.c_str(), ENUM_CURRENT_SETTINGS, &mode, 0)) {
        return static_cast<int>(mode.dmDisplayFrequency);
    }
    return 0;
}

HRESULT createSceneRenderer(
    IDXGISwapChain* swapChain,
    ID2D1RenderTarget** d2dRenderTarget,
    IDWriteTextFormat** titleFormat,
    IDWriteTextFormat** bodyFormat,
    ID2D1SolidColorBrush** whiteBrush,
    ID2D1SolidColorBrush** accentBrush,
    ID2D1SolidColorBrush** panelBrush) {
    ID2D1Factory* rawD2dFactory = nullptr;
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &rawD2dFactory);
    if (FAILED(hr)) return hr;
    ComPtr<ID2D1Factory> d2dFactory;
    d2dFactory.Attach(rawD2dFactory);

    ComPtr<IDXGISurface> surface;
    hr = swapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf()));
    if (FAILED(hr)) return hr;

    D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f,
        96.0f);
    hr = d2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &properties, d2dRenderTarget);
    if (FAILED(hr)) return hr;

    ComPtr<IDWriteFactory> writeFactory;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(writeFactory.GetAddressOf()));
    if (FAILED(hr)) return hr;

    hr = writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        64.0f,
        L"en-us",
        titleFormat);
    if (FAILED(hr)) return hr;
    (*titleFormat)->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    (*titleFormat)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    hr = writeFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        34.0f,
        L"en-us",
        bodyFormat);
    if (FAILED(hr)) return hr;
    (*bodyFormat)->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    (*bodyFormat)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    hr = (*d2dRenderTarget)->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), whiteBrush);
    if (FAILED(hr)) return hr;
    hr = (*d2dRenderTarget)->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.80f, 0.0f, 1.0f), accentBrush);
    if (FAILED(hr)) return hr;
    hr = (*d2dRenderTarget)->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.64f, 1.0f, 0.85f), panelBrush);
    return hr;
}

HRESULT renderSharedScene(
    ID2D1RenderTarget* renderTarget,
    IDWriteTextFormat* titleFormat,
    IDWriteTextFormat* bodyFormat,
    ID2D1SolidColorBrush* whiteBrush,
    ID2D1SolidColorBrush* accentBrush,
    ID2D1SolidColorBrush* panelBrush,
    int frameCount) {
    const D2D1_SIZE_F size = renderTarget->GetSize();
    const float phase = static_cast<float>((frameCount % 240) / 240.0);
    renderTarget->BeginDraw();
    renderTarget->Clear(D2D1::ColorF(0.04f + 0.04f * phase, 0.07f, 0.11f + 0.05f * (1.0f - phase), 1.0f));

    const float panelWidth = std::min(size.width * 0.82f, 1100.0f);
    const float panelHeight = 270.0f;
    const float panelLeft = (size.width - panelWidth) / 2.0f;
    const float panelTop = (size.height - panelHeight) / 2.0f;
    const D2D1_RECT_F panel = D2D1::RectF(panelLeft, panelTop, panelLeft + panelWidth, panelTop + panelHeight);
    renderTarget->FillRectangle(panel, panelBrush);

    const D2D1_RECT_F title = D2D1::RectF(panelLeft, panelTop + 50.0f, panelLeft + panelWidth, panelTop + 130.0f);
    const wchar_t* titleText = L"AR Overlay Ready";
    renderTarget->DrawText(titleText, static_cast<UINT32>(std::wcslen(titleText)), titleFormat, title, whiteBrush);

    const D2D1_RECT_F body = D2D1::RectF(panelLeft, panelTop + 150.0f, panelLeft + panelWidth, panelTop + 215.0f);
    const wchar_t* bodyText = L"Shared scene via Windows DXGI";
    renderTarget->DrawText(bodyText, static_cast<UINT32>(std::wcslen(bodyText)), bodyFormat, body, accentBrush);

    const D2D1_RECT_F footer = D2D1::RectF(panelLeft, panelTop + 215.0f, panelLeft + panelWidth, panelTop + 258.0f);
    const wchar_t* footerText = L"Host input and API commands remain the shared input path";
    renderTarget->DrawText(footerText, static_cast<UINT32>(std::wcslen(footerText)), bodyFormat, footer, whiteBrush);

    return renderTarget->EndDraw();
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
            swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
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
                D3D11_CREATE_DEVICE_BGRA_SUPPORT,
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

    ComPtr<ID2D1RenderTarget> d2dRenderTarget;
    ComPtr<IDWriteTextFormat> titleFormat;
    ComPtr<IDWriteTextFormat> bodyFormat;
    ComPtr<ID2D1SolidColorBrush> whiteBrush;
    ComPtr<ID2D1SolidColorBrush> accentBrush;
    ComPtr<ID2D1SolidColorBrush> panelBrush;
    hr = createSceneRenderer(
        swapChain.Get(),
        d2dRenderTarget.GetAddressOf(),
        titleFormat.GetAddressOf(),
        bodyFormat.GetAddressOf(),
        whiteBrush.GetAddressOf(),
        accentBrush.GetAddressOf(),
        panelBrush.GetAddressOf());
    if (FAILED(hr)) {
        std::cerr << "createSceneRenderer failed: " << hresultHex(hr) << "\n";
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
        << "\"scene\":\"static-text\","
        << "\"renderer\":\"direct2d-directwrite\","
        << "\"durationSeconds\":" << durationSeconds
        << "}" << std::endl;

    const int targetRefreshHz = refreshRateForMonitor(target);
    const double targetFrameMs = targetFrameMsForRefresh(targetRefreshHz);
    const auto startedAt = std::chrono::steady_clock::now();
    const auto stopAt = startedAt + std::chrono::seconds(durationSeconds);
    auto previousFrameAt = startedAt;
    FrameStats stats;
    MSG msg{};
    while (running && std::chrono::steady_clock::now() < stopAt) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        hr = renderSharedScene(
            d2dRenderTarget.Get(),
            titleFormat.Get(),
            bodyFormat.Get(),
            whiteBrush.Get(),
            accentBrush.Get(),
            panelBrush.Get(),
            stats.frameCount);
        if (FAILED(hr)) {
            std::cerr << "renderSharedScene failed: " << hresultHex(hr) << "\n";
            break;
        }

        hr = swapChain->Present(1, 0);
        if (FAILED(hr)) {
            ++stats.presentFailures;
            stats.lastPresentFailure = hr;
        }

        const auto now = std::chrono::steady_clock::now();
        recordFrameInterval(stats, std::chrono::duration<double, std::milli>(now - previousFrameAt).count(), targetFrameMs);
        previousFrameAt = now;
        ++stats.frameCount;
    }

    const auto endedAt = std::chrono::steady_clock::now();
    if (stats.frameCount > 0) {
        stats.avgFrameMs /= static_cast<double>(stats.frameCount);
    }
    std::cout
        << "{"
        << "\"platform\":\"windows\","
        << "\"mode\":\"xreal-dxgi-preview\","
        << "\"event\":\"summary\","
        << "\"scene\":\"static-text\","
        << "\"frameCount\":" << stats.frameCount << ","
        << "\"elapsedMs\":" << std::chrono::duration<double, std::milli>(endedAt - startedAt).count()
        << ",\"targetRefreshHz\":" << targetRefreshHz
        << ",\"targetFrameMs\":" << targetFrameMs
        << ",\"minFrameMs\":" << stats.minFrameMs
        << ",\"avgFrameMs\":" << stats.avgFrameMs
        << ",\"maxFrameMs\":" << stats.maxFrameMs
        << ",\"framesOver20Ms\":" << stats.framesOver20Ms
        << ",\"framesOver33Ms\":" << stats.framesOver33Ms
        << ",\"framesOverTargetBy2Ms\":" << stats.framesOverTargetBy2Ms
        << ",\"presentFailures\":" << stats.presentFailures
        << ",\"lastPresentFailure\":\"" << hresultHex(stats.lastPresentFailure) << "\""
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
