================================================
FILE: readme.md
================================================

# testProject

A minimal C/C++ project scaffolded with CMake, supporting both native and WebAssembly (WASM) builds. Includes a helper script (`build_and_run.sh`) to configure, build, and either run the native executable or serve the WASM build in your browser, now with WebGL2 rendering support.

---

## Table of Contents

1. [Features](#features)
2. [Prerequisites](#prerequisites)
3. [Getting Started](#getting-started)
4. [Building & Running](#building--running)
5. [Script Options](#script-options)
6. [WebAssembly + WebGL Build Notes](#webassembly--webgl-build-notes)
7. [Project Structure](#project-structure)
8. [License](#license)

---

## Features

- **Native Build** via CMake (+ Ninja or Make).
- **WebAssembly Build** via Emscripten, with WebGL2 context creation.
- Auto-generates a simple `index.html` for WASM builds (with `<canvas>`).
- Simple C/C++ rendering module (`render.[ch]`) that clears the screen each frame.
- Serves WASM build on `http://localhost:8000`, with auto-browser launch and port-conflict handling.

---

## Prerequisites

- **Bash** (Linux, macOS, or WSL)
- **CMake** ≥ 3.0
- **Ninja** _or_ **Make** (build system)
- **Python 3** (for serving WASM builds)
- **lsof** (optional; for port checks)
- **Emscripten SDK** (with OpenGL ES / WebGL support enabled)

---

## Getting Started

```bash
git clone https://your.repo.url/testProject.git
cd testProject
chmod +x build_and_run.sh
```
