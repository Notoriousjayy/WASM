# testProject

A minimal C project scaffolded with CMake, supporting both native and WebAssembly (WASM) builds. Includes a helper script (`build_and_run.sh`) to configure, build, and either run the native executable or serve the WASM build in your browser.

---

## Table of Contents

1. [Features](#features)
2. [Prerequisites](#prerequisites)
3. [Getting Started](#getting-started)
   - [Clone the Repository](#clone-the-repository)
   - [Directory Structure](#directory-structure)
4. [Building & Running](#building--running)
   - [Using `build_and_run.sh`](#using-build_and_runsh)
   - [Manual CMake Build](#manual-cmake-build)
5. [Script Options](#script-options)
6. [WebAssembly Build Notes](#webassembly-build-notes)
7. [License](#license)

---

## Features

- **Native Build** via CMake (+ Ninja or Make).
- **WebAssembly Build** via Emscripten toolchain.
- Auto-generates a simple `index.html` for WASM builds.
- Serves WASM build on `http://localhost:8000`, with automatic browser launch and port-conflict handling.

---

## Prerequisites

- **Bash** (on Linux, macOS, or WSL)
- **CMake** ≥ 3.0
- **Ninja** _or_ **Make** (for the build system)
- **Python 3** (for serving WASM builds)
- **lsof** (optional; used to check/kill processes on port 8000)
- **Emscripten SDK** (for WASM builds)

---

## Getting Started

### Clone the Repository

```bash
git clone https://your.repo.url/testProject.git
cd testProject
chmod +x build_and_run.sh
```
