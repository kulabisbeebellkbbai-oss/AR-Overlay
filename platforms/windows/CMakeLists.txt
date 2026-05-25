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

if(WIN32)
    target_link_libraries(ar-overlay-windows-preview PRIVATE user32 gdi32 shcore winmm)
endif()
