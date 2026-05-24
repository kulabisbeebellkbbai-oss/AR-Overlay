cmake_minimum_required(VERSION 3.20)
project(ar_overlay_linux_smoke LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ar-overlay-linux-smoke src/main.cpp)
