cmake_minimum_required(VERSION 3.0)
project(testProject C)

# Use C99
set(CMAKE_C_STANDARD 99)

# To build for WebAssembly:
#   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/home/jordan/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
# For native testing, omit -DCMAKE_TOOLCHAIN_FILE.

add_executable(testProject src/main.c)
