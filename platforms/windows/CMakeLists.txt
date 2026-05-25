cmake_minimum_required(VERSION 3.20)
project(ar_overlay_windows_smoke LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ar-overlay-windows-smoke
    src/main.cpp
    src/WindowsAdapter.cpp
    src/CapabilityProbe.cpp
    src/FixtureRunner.cpp
)

target_include_directories(ar-overlay-windows-smoke PRIVATE src)

add_executable(ar-overlay-windows-preview
    src/preview.cpp
)

target_include_directories(ar-overlay-windows-preview PRIVATE src)

add_executable(ar-overlay-windows-dxgi-preview
    src/dxgi_preview.cpp
)

target_include_directories(ar-overlay-windows-dxgi-preview PRIVATE src)

add_executable(ar-overlay-windows-raw-input-capture
    src/raw_input_capture.cpp
)

target_include_directories(ar-overlay-windows-raw-input-capture PRIVATE src)

if(WIN32)
    target_link_libraries(ar-overlay-windows-preview PRIVATE user32 gdi32 shcore winmm)
    target_link_libraries(ar-overlay-windows-dxgi-preview PRIVATE user32 shcore d3d11 dxgi d2d1 dwrite)
    target_link_libraries(ar-overlay-windows-raw-input-capture PRIVATE user32)
endif()
