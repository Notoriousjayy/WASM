# Build Guide

This project supports multiple build configurations for both native platforms (using SDL3) and WebAssembly (using Emscripten).

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Build Configurations](#build-configurations)
- [Platform-Specific Notes](#platform-specific-notes)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Common Requirements

- **CMake** 3.20 or higher
- **Ninja** build system (recommended) or Make
- **C99-compatible compiler** (GCC, Clang, MSVC)

### For Native Builds (SDL3)

SDL3 will be automatically downloaded and built if not found on your system.

**Optional: Manual SDL3 Installation**

#### Linux (Ubuntu/Debian)
```bash
# Build from source (recommended)
git clone https://github.com/libsdl-org/SDL.git
cd SDL
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

#### macOS
```bash
# Using Homebrew
brew install sdl3

# Or build from source
git clone https://github.com/libsdl-org/SDL.git
cd SDL
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
sudo make install
```

#### Windows
```powershell
# Using vcpkg
vcpkg install sdl3:x64-windows

# Or download prebuilt binaries from:
# https://github.com/libsdl-org/SDL/releases
```

### For WebAssembly Builds

- **Emscripten SDK** (emsdk)

```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

---

## Quick Start

### Native Build (SDL3 + OpenGL)

```bash
# Configure
cmake --preset native-sdl3-debug

# Build
cmake --build --preset native-sdl3-debug

# Run
./build-native-debug/bin/testProject
```

### WebAssembly Build

```bash
# Activate Emscripten environment
source /path/to/emsdk/emsdk_env.sh

# Configure
emcmake cmake --preset wasm-debug

# Build
cmake --build --preset wasm-debug

# Serve locally
cmake --build build-wasm-debug --target serve
# Open http://localhost:8000 in your browser
```

---

## Build Configurations

### Available Presets

| Preset | Platform | Graphics | Build Type | Description |
|--------|----------|----------|------------|-------------|
| `wasm-debug` | WebAssembly | WebGL2 | Debug | WASM with debug symbols and assertions |
| `wasm-release` | WebAssembly | WebGL2 | Release | Optimized WASM for production |
| `native-sdl3-debug` | Native | SDL3+OpenGL | Debug | Native build with debugging |
| `native-sdl3-release` | Native | SDL3+OpenGL | Release | Optimized native build |
| `native-headless` | Native | None | Debug | Console-only build (no graphics) |
| `native-tests` | Native | SDL3+OpenGL | Debug | Build with unit tests enabled |

### Using Presets

```bash
# List all available presets
cmake --list-presets

# Configure with a preset
cmake --preset <preset-name>

# Build with a preset
cmake --build --preset <preset-name>

# Run tests (for native-tests preset)
ctest --preset native-tests
```

---

## Platform-Specific Notes

### Linux

**Required system packages:**
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake ninja-build \
                 libgl1-mesa-dev libxext-dev

# Fedora/RHEL
sudo dnf install gcc gcc-c++ cmake ninja-build \
                 mesa-libGL-devel libXext-devel

# Arch Linux
sudo pacman -S base-devel cmake ninja mesa
```

**Building:**
```bash
cmake --preset native-sdl3-debug
cmake --build --preset native-sdl3-debug -j$(nproc)
./build-native-debug/bin/testProject
```

### macOS

**Required tools:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake and Ninja (via Homebrew)
brew install cmake ninja
```

**Building:**
```bash
cmake --preset native-sdl3-debug
cmake --build --preset native-sdl3-debug -j$(sysctl -n hw.ncpu)
./build-native-debug/bin/testProject
```

### Windows

**Using Visual Studio 2022:**
```powershell
# Configure (Visual Studio will be auto-detected)
cmake -B build -DUSE_SDL3=ON

# Build
cmake --build build --config Debug

# Run
.\build\bin\Debug\testProject.exe
```

**Using MinGW/MSYS2:**
```bash
# In MSYS2 shell
cmake --preset native-sdl3-debug -G "MinGW Makefiles"
cmake --build --preset native-sdl3-debug -j$(nproc)
./build-native-debug/bin/testProject.exe
```

### WebAssembly

**Building:**
```bash
# Set up Emscripten environment
source /path/to/emsdk/emsdk_env.sh

# Configure
emcmake cmake --preset wasm-debug

# Build
cmake --build --preset wasm-debug

# Output files will be in build-wasm-debug/
# - index.html (entry point)
# - index.js (JavaScript glue code)
# - index.wasm (WebAssembly binary)
```

**Local testing:**
```bash
# Start development server (requires Python 3)
cmake --build build-wasm-debug --target serve

# Or use any static file server:
cd build-wasm-debug
python3 -m http.server 8000
# Open http://localhost:8000
```

---

## Advanced Build Options

### Custom Build Configurations

```bash
# Disable SDL3 (console-only build)
cmake -B build -DUSE_SDL3=OFF

# Custom server port for WASM builds
cmake --preset wasm-debug -DSERVE_PORT=3000

# Enable tests
cmake --preset native-tests -DENABLE_TESTS=ON

# Custom SDL3 installation path
cmake --preset native-sdl3-debug -DCMAKE_PREFIX_PATH=/custom/sdl3/path
```

### Optimization Levels

**Native:**
```bash
# Debug with sanitizers (Linux/macOS)
cmake -B build-sanitized -DUSE_SDL3=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-fsanitize=address,undefined"

# Release with LTO
cmake -B build-lto -DUSE_SDL3=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

**WebAssembly:**
```bash
# Maximum optimization
emcmake cmake --preset wasm-release \
              -DCMAKE_C_FLAGS="-O3 -flto"

# Smallest binary size
emcmake cmake --preset wasm-release \
              -DCMAKE_C_FLAGS="-Oz -flto"
```

---

## Troubleshooting

### SDL3 Not Found

**Error:** `SDL3 not found locally, fetching from GitHub...`

**Solution:** This is normal. CMake will automatically download and build SDL3. If you want to use a system-installed version:

```bash
# Install SDL3 first, then:
cmake --preset native-sdl3-debug -DCMAKE_PREFIX_PATH=/usr/local
```

### Emscripten Errors

**Error:** `emcmake: command not found`

**Solution:**
```bash
# Make sure Emscripten is activated
source /path/to/emsdk/emsdk_env.sh

# Verify installation
emcc --version
```

**Error:** `WebGL context creation failed`

**Solution:**
- Ensure your browser supports WebGL2
- Check browser console for detailed errors
- Try a different browser (Chrome, Firefox, Edge recommended)

### OpenGL Errors (Native)

**Error:** `Failed to create OpenGL context`

**Solution:**
```bash
# Linux: Install mesa drivers
sudo apt install mesa-utils
glxinfo | grep "OpenGL version"

# macOS: Update to latest OS version
# Windows: Update graphics drivers
```

### Build Errors

**Error:** `No CMAKE_C_COMPILER could be found`

**Solution:**
```bash
# Linux
sudo apt install build-essential

# macOS
xcode-select --install

# Windows
# Install Visual Studio with C++ workload
```

**Error:** `Ninja not found`

**Solution:**
```bash
# Option 1: Install Ninja
sudo apt install ninja-build  # Linux
brew install ninja            # macOS

# Option 2: Use Make instead
cmake --preset native-sdl3-debug -G "Unix Makefiles"
```

---

## Testing

```bash
# Build with tests enabled
cmake --preset native-tests

# Run all tests
ctest --preset native-tests

# Run specific test
ctest --preset native-tests -R polygon_tests

# Verbose output
ctest --preset native-tests --output-on-failure
```

---

## Continuous Integration

The project includes GitHub Actions workflows:

- **build-wasm.yml** - Builds WebAssembly artifacts
- **codeql.yml** - Security analysis
- **pages.yml** - Deploys to GitHub Pages
- **release.yml** - Creates release packages

See `.github/workflows/` for details.

---

## Clean Build

```bash
# Remove build directories
rm -rf build-*

# Or clean specific configuration
cmake --build build-native-debug --target clean
```

---

## Additional Resources

- [SDL3 Documentation](https://wiki.libsdl.org/SDL3/)
- [Emscripten Documentation](https://emscripten.org/docs/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Project Issues](https://github.com/yourusername/yourrepo/issues)