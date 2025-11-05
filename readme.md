# WASM Native Game Starter

Minimal C/C++ scaffold built with **CMake**, targeting **WebAssembly (Emscripten + WebGL2)** and optionally native. Ideal for prototyping graphics/game code that runs in the browser while keeping a clean, production-style CMake project.

<p align="left">
  <a href="https://opensource.org/licenses/MIT"><img alt="License" src="https://img.shields.io/badge/License-MIT-green.svg"></a>
</p>

---

## Why

Build a **native-performance browser game** using **C/C++** compiled to **WebAssembly (WASM)**. This starter pairs **Emscripten** with **WebGL2** so you can write low-level, deterministic rendering and game logic in C/C++ and ship it to the web without rewriting in JavaScript. You keep your normal CMake workflow, and the browser becomes a high-performance runtime.

---

## Quick Start (WASM)

### 0) Prerequisites

* CMake ≥ 3.20
* Python 3 (for the dev server)
* Emscripten SDK

  ```bash
  git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
  cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest
  source ~/emsdk/emsdk_env.sh
  ```
* (Recommended) Ninja

  ```bash
  sudo apt-get update && sudo apt-get install -y ninja-build
  ```

### 1) Configure & build (presets)

```bash
# Configure for WASM
emcmake cmake --preset wasm-debug

# Build
cmake --build --preset wasm-debug
```

### 2) Serve locally

```bash
# Runs `python3 -m http.server` from the build dir
cmake --build --preset wasm-debug --target serve
```

Open [http://localhost:8000/](http://localhost:8000/).
The build emits `index.html / index.js / index.wasm` in the build directory, so your app loads at the server root.

> WSL tip: the server binds to `0.0.0.0`, so you can open the URL from Windows too.

---

## Features

* **CMake-only workflow**: no bespoke shell scripts required.
* **WebGL2** rendering via Emscripten HTML5 APIs.
* **Custom HTML shell (optional)** with a single `{{{ SCRIPT }}}` placeholder.
* **Tiny runtime hook**: initializes WebGL and starts a main loop.
* **Exported function** `myFunction()` callable from DevTools:

  ```js
  Module.ccall('myFunction', null, [], []);
  ```

---

## Build Matrix

### With presets (recommended)

```bash
# WASM (Debug)
emcmake cmake --preset wasm-debug
cmake --build --preset wasm-debug
cmake --build --preset wasm-debug --target serve
```

### Without presets

```bash
emcmake cmake -S . -B build-wasm -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-wasm
(cd build-wasm && python3 -m http.server 8000)
```

> Native builds: `src/render.c` is WebGL2/Emscripten-only. For native, add a `render_native.c` (e.g., GLFW/SDL) and wire it in CMake for non-Emscripten targets.

---

## Configuration

CMake cache variables:

| Option / Var      |  Default  | Purpose                                                   |
| ----------------- | :-------: | --------------------------------------------------------- |
| `BUILD_WASM_HTML` |    `ON`   | Use custom HTML shell if present, else Emscripten default |
| `SERVE_PORT`      |   `8000`  | Port for `serve` target                                   |
| `SERVE_HOST`      | `0.0.0.0` | Bind address for `serve` target                           |

Override at configure time:

```bash
emcmake cmake --preset wasm-debug -DSERVE_PORT=5173 -DSERVE_HOST=127.0.0.1
```

### Custom HTML Shell

If `html_template/index.html` exists, it is used automatically and must contain:

```html
{{{ SCRIPT }}}
```

You can optionally add `html_template/init_runtime.js` to place your `Module.onRuntimeInitialized` logic outside the HTML.

---

## Project Layout

```
.
├─ CMakeLists.txt
├─ CMakePresets.json         # presets for wasm-debug, etc.
├─ include/
│  └─ testProject/
│     ├─ render.h
│     └─ module.h
├─ src/
│  ├─ main.c                 # entry point
│  ├─ render.c               # WebGL2 (Emscripten) renderer
│  └─ module.c               # EMSCRIPTEN_KEEPALIVE myFunction()
└─ html_template/            # (optional) custom shell / post-js
   └─ index.html
```

---

## API Reference

### `int initWebGL(void)`

Creates a WebGL2 context on `<canvas id="canvas">`.
**Returns**: `1` on success, `0` on failure.

### `void startMainLoop(void)`

Starts the frame loop (clears to a solid color by default).

### `void myFunction(void)`

Tagged `EMSCRIPTEN_KEEPALIVE`. Call from JS:

```js
Module.ccall('myFunction', null, [], []);
```

---

## CI/CD

This repository includes GitHub Actions workflows:

* **Build (WASM C/C++)**: compiles the WASM artifact on push/PR.
* **CodeQL (C/C++)**: static analysis on schedule and PRs.
* **Dependency Review**: flags vulnerable dependencies in PRs.
* **Deploy (GitHub Pages)**: builds the site and publishes to Pages.
* **Release**: on tags `v*.*.*`, builds artifacts and attaches them to a GitHub Release.

> Ensure **GitHub Pages** is enabled in repository settings if you plan to deploy.

---

## Roadmap

| Area      | Item                                        | Notes                                     |
| --------- | ------------------------------------------- | ----------------------------------------- |
| Rendering | Fragment shader color/gradient paramization | Uniforms + simple UI to tweak             |
| Input     | Keyboard/mouse and touch handling           | Emscripten HTML5 callbacks                |
| Timing    | Fixed-timestep update + variable render     | Deterministic game logic                  |
| Audio     | Minimal SFX pipeline                        | WebAudio interop layer                    |
| Assets    | Basic packer/loader                         | `.data` packaging with CMake step         |
| Gameplay  | Collision primitives                        | Circle/box first, then SAT                |
| Arch      | Scene/state management                      | Pushdown automaton or ECS-lite            |
| Native    | GLFW/SDL path                               | `render_native.c` + desktop build preset  |
| Testing   | Headless render sanity checks               | Offscreen canvas/CI probe                 |
| CI/CD     | Pages smoke test                            | `curl`/`playwright` ping of deployed site |

---

## Troubleshooting

* **Ninja not found**
  `CMake was unable to find a build program corresponding to "Ninja"`
  → `sudo apt-get install -y ninja-build` or use `-G "Unix Makefiles"`.

* **Custom shell error**: `HTML shell must contain {{{ SCRIPT }}}`
  → Add the placeholder to `html_template/index.html`, or use the default shell.

* **Black/blank canvas**

  * Check DevTools console for WebGL errors.
  * Ensure the browser supports **WebGL2** and **WASM**.
  * Verify `initWebGL()` returns `1`.

* **WSL cannot reach server**
  Use `-DSERVE_HOST=0.0.0.0` and open `http://localhost:<port>` from Windows.

---

## Contributing

PRs welcome. Please open an issue first for significant changes.

---

## License

[MIT](https://opensource.org/licenses/MIT)
