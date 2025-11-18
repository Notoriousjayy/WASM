# WASM Native Game Starter

Minimal C-based scaffold for a WebAssembly **game / graphics / math testbed** that can also be compiled natively.

The project is designed as a small but serious playground for:

- WebAssembly + WebGL2 rendering
- A Cyclone-style physics core with configurable precision
- Vector / matrix math utilities
- Polygon and polynomial modules for computational geometry and numerical experiments
- A C-based numerical calculus library inspired by TAOCP Vol. 2 + Numerical Recipes
- CI/CD pipelines (build, Pages, release zips, CodeQL, dependency review)

---

## Goals

- **Clean split** between “core logic” and “rendering”
- **Single CMake-based toolchain** that works for both WASM and native builds
- Easy to integrate into **CI/CD** (artifacts, GitHub Pages, Release zips)
- **Serious C layout** while staying small and hackable
- Serve as a **sandbox** for graphics, math, and physics experiments (Pong, polygon demos, numerical experiments, etc.)

---

## Project Layout

Top-level:

- `CMakeLists.txt` – project definition, targets, and Emscripten-specific options
- `CMakePresets.json` – presets for `wasm-debug` and `native-debug`
- `Dockerfile` – multi-stage build (Emscripten builder + nginx static host)
- `package.json` – optional Node dev servers (`http-server`, `live-server`)
- `html_template/index.html` – custom Emscripten HTML shell for WebGL2
- `.github/workflows/` – CI/CD pipelines
  - `build-wasm.yml` – build & upload WASM artifacts on push/PR
  - `pages.yml` – deploy to GitHub Pages from `main`
  - `release.yml` – build and zip release (`pong-wasm.zip`) on tags
  - `codeql.yml` – CodeQL security analysis (C/C++)
  - `dependency-review.yml` – GitHub Dependency Review

---

### Core game / graphics project

Core headers (under `include/testProject/`):

- `precision.h`  
  - Defines the `real` type and wrapper functions (`real_sqrt`, `real_sin`, etc.).
  - Allows swapping precision modes for the physics core.

- `core.h`  
  - Cyclone-style physics core utilities:
    - 3D vectors, matrices, quaternions
    - Transforms and basic rigid-body math helpers
  - Intended foundation for future physics-driven gameplay.

- `vectors.h`  
  - 2D/3D vector types (`vec2`, `vec3`, `vec4`) and ops:
    - add/subtract, scalar multiply, dot product
    - magnitude / normalization
    - cross product (3D)
    - angle between vectors
    - projection / perpendicular / reflection

- `matrices.h`  
  - 2×2, 3×3, 4×4 matrices:
    - row-major layout
    - identity / multiplication
    - transform creation (translate/rotate/scale)
    - projection / orthographic matrices
  - Helpers to bridge into WebGL-friendly float arrays.

- `polygon.h`  
  - Dynamic polygon structure:
    - manage vertices
    - translate / rotate / scale
    - compute perimeter
    - export flat float arrays for WebGL buffers
  - Used by the WebGL renderer for regular-ngon demos (e.g., moving hexagon).

- `polynomial.h`  
  - Polynomial representation & operations:
    - construct polynomials
    - evaluate `p(x)`
    - add / multiply
  - Useful for curve definitions, animation easing, or general math experiments.

- `render.h`  
  - Declares WebGL-facing entry points:
    - `void initWebGL(void);`
    - `void startMainLoop(void);`
  - These are exported to JavaScript via Emscripten and called from `index.html`.

- `module.h`  
  - Example Emscripten-exported function `myFunction` to demonstrate JS ↔ C interop.

Implementation files (under `src/`):

- `main.c`  
  - Entry point for the application.
  - Sets up core state and (for WASM builds) delegates to `initWebGL()` / `startMainLoop()`.

- `core.c`, `precision.c`  
  - Implement parts of the Cyclone-style physics and precision helpers.

- `vectors.c`, `matrices.c`  
  - Implement the vector and matrix operations declared in the headers.

- `polygon.c`, `polynomial.c`  
  - Implement polygon management and polynomial math.

- `render.c`  
  - **WASM-only** renderer (compiled only when using Emscripten):
    - Initializes WebGL2 context via bindings from `index.html`
    - Builds polygon geometry, uploads vertex buffers
    - Compiles shaders and runs the main animation loop

- `module.c`  
  - Implements the exported sample function `myFunction`.

> Note: For native builds, `render.c` is excluded. Native rendering is intentionally left as “future work” – the core math & physics compiles, but there is no OpenGL/SDL/etc. renderer yet.

---

### Numerical Calculus Library (C)

The numerical calculus library is a **modular C library** inspired by TAOCP Vol. 2 and Numerical Recipes. It is split into:

- Header files under `include/mathlib/<module>/`
- Implementation files under `src/mathlib/<module>/`

Each domain has a clear header/source pairing.

#### Core floating point + polynomials

- Headers (`include/mathlib/core/`):
  - `fp.h` – floating-point helpers, `Real` typedef, error metrics (`machine_epsilon`, `ulp`, `nearly_equal`, robust summation).
  - `poly.h` – polynomial representation and evaluation; basic operations.

- Sources (`src/mathlib/core/`):
  - `fp.c` – implementation of floating-point helpers and robust summation.
  - `poly.c` – implementation of polynomial evaluation and operations.

#### Random numbers & Monte Carlo

- Header (`include/mathlib/rand/`):
  - `rand.h` – PRNG core, uniform/normal distributions, Monte Carlo helpers.

- Source (`src/mathlib/rand/`):
  - `rand.c` – implementation of RNG state, generators, and distributions.

#### Interpolation & approximation

- Header (`include/mathlib/interp/`):
  - `interp.h` – 1D interpolation tables, polynomial interpolation, spline interpolation, Chebyshev-based approximation helpers.

- Source (`src/mathlib/interp/`):
  - `interp.c` – implementation of table lookup, interpolation, and Chebyshev routines.

#### Differentiation & integration (diffint)

- Headers (`include/mathlib/diffint/`):
  - `deriv.h` – numerical differentiation (forward/central differences, Richardson extrapolation).
  - `quad.h` – quadrature (trapezoid, Simpson, Romberg, Gauss–Legendre, adaptive integration).

- Sources (`src/mathlib/diffint/`):
  - `deriv.c` – implementation of derivative routines where not inlined.
  - `quad.c` – implementation of quadrature and adaptive integration.

#### Linear algebra backbone

- Header (`include/mathlib/linalg/`):
  - `linalg.h` – dense vector/matrix types, row-major storage, LU and Cholesky solves.

- Source (`src/mathlib/linalg/`):
  - `linalg.c` – implementation of factorization and solve routines.

#### Nonlinear equations / root finding

- Header (`include/mathlib/nonlin/`):
  - `root.h` – scalar root-finding interfaces (bisection, Newton, Brent-style) and small-system helpers.

- Source (`src/mathlib/nonlin/`):
  - `root.c` – implementation of root finders and system solvers using `linalg`.

#### Optimization

- Header (`include/mathlib/optim/`):
  - `optim.h` – 1D minimization (golden-section, Brent) and multi-dimensional minimization interfaces (e.g., conjugate gradient / quasi-Newton).

- Source (`src/mathlib/optim/`):
  - `optim.c` – implementation of line search and iterative optimization routines.

#### Spectral / FFT tools

- Header (`include/mathlib/spectral/`):
  - `fft.h` – FFT interface, inverse FFT, and spectral differentiation/integration primitives.

- Source (`src/mathlib/spectral/`):
  - `fft.c` – implementation of FFT/ifft and related spectral operations.

#### ODE solvers (initial value & boundary value)

- Headers (`include/mathlib/ode/`):
  - `ivp.h` – initial value problem solvers (Runge–Kutta, adaptive RK).
  - `bvp.h` – boundary value problem scaffolding (shooting / relaxation).

- Sources (`src/mathlib/ode/`):
  - `ivp.c` – implementation of IVP time-stepping schemes.
  - `bvp.c` – implementation of BVP shooting / relaxation helpers.

#### PDE solvers

- Header (`include/mathlib/pde/`):
  - `pde.h` – simple finite-difference PDE solvers (e.g., 1D heat equation).

- Source (`src/mathlib/pde/`):
  - `pde.c` – implementation of PDE time-stepping and spatial discretization.

#### Statistics & error analysis

- Header (`include/mathlib/stats/`):
  - `stats.h` – mean, variance, covariance, and simple least-squares fitting helpers.

- Source (`src/mathlib/stats/`):
  - `stats.c` – implementation of statistical routines used by Monte Carlo and error analysis.

The intent is to keep these modules **plain C and reusable**:

- You can link them into the WASM/WebGL demos in this repo.
- You can link them into native test harnesses or other projects.
- Physics/gameplay systems can depend on well-factored numerical primitives without pulling in rendering concerns.

---

## Build & Run (WASM via CMake)

### Prerequisites

* Emscripten SDK (emsdk)
* CMake ≥ 3.20
* Ninja (recommended)
* A recent C compiler (Clang/GCC)

### Configure & build (WASM)

From the repo root:

```bash
# Configure (WASM debug)
emcmake cmake --preset wasm-debug

# Build
cmake --build --preset wasm-debug -j2
````

Artifacts will be in `build-wasm/`:

* `index.html`
* `index.js`
* `index.wasm`
* Optional `index.data` (if used)

### Serve locally

You have multiple options:

1. **CMake + emrun (configured in CMakeLists):**

   ```bash
   cmake --build --preset wasm-debug --target serve
   ```

   This runs:

   ```bash
   emrun --no_browser --port 8000 build-wasm/index.html
   ```

   Then open: `http://localhost:8000/`.

2. **Node dev servers (from `package.json`):**

   ```bash
   npm install
   npx http-server build-wasm -p 8000
   # or
   npx live-server build-wasm
   ```

3. **Docker (see next section)**

---

## Build & Run with Docker

The repo ships with a multi-stage Dockerfile that:

1. Uses `emscripten/emsdk` to build the WASM artifacts.
2. Copies the contents of `build-wasm/` into an `nginx:alpine` image.

Build and run:

```bash
# Build the image
docker build -t wasm-game .

# Run the container
docker run --rm -p 8080:80 wasm-game
```

Then open: `http://localhost:8080/`.

This is useful for:

* Quickly sharing the build without local toolchain setup.
* Running the project behind a standard nginx static host.
* Serving as a starting point for container-based deployment (e.g., GHCR + k8s).

> CI workflows currently build **directly with Emscripten on the runner** and do not use this Dockerfile yet. You can integrate Docker into the workflows later if you prefer containerized builds.

---

## Native Build (Experimental)

There is a `native-debug` preset for compiling the core without Emscripten:

```bash
cmake --preset native-debug
cmake --build --preset native-debug -j2
```

This will:

* Build the **core math / physics / utility modules** as a native binary.
* **Not** compile `render.c` (no native renderer is wired up yet).

Use this if you want to:

* Run unit tests against the math/physics modules.
* Prototype native backends (e.g., SDL2 + OpenGL).

---

## CI/CD Overview

All workflows live under `.github/workflows/`:

* **Build (WASM) – `build-wasm.yml`**

  * Trigger: push / pull_request to `main`.
  * Installs Emscripten, caches build cache.
  * Configures and builds using `wasm-debug` preset.
  * Uploads the `build-wasm` artifacts.

* **GitHub Pages – `pages.yml`**

  * Trigger: push to `main`.
  * Builds with Emscripten.
  * Copies `build-wasm/index.*` into a `site/` dir.
  * Uses `actions/deploy-pages` to publish to GitHub Pages.

* **Release – `release.yml`**

  * Trigger: tags matching `v*.*.*`.
  * Builds with Emscripten.
  * Packages `index.html/js/wasm[/data]` into `release/pong-wasm.zip`.
  * Creates a GitHub Release with the zip attached.

* **CodeQL – `codeql.yml`**

  * Trigger: push, PRs to `main`, scheduled weekly.
  * Builds the project using Emscripten and runs CodeQL analysis for C/C++.

* **Dependency Review – `dependency-review.yml`**

  * Trigger: PR opened / synchronized / reopened.
  * Runs GitHub’s dependency review action.

This layout gives you:

* Automatic build validation on every push/PR.
* Continuous deployment to GitHub Pages from `main`.
* Tagged releases that bundle the static WASM site.
* Security analysis and supply-chain checks.

---

## Extending the Project

A few common extension points:

* **Add new math / physics features**

  * Extend `vectors.h` / `matrices.h` for more operations.
  * Add new components to `core.h` or additional physics modules.
  * Implement new numerical primitives inside the math library (integration methods, ODE solvers, optimizers).

* **Add new geometry demos**

  * Extend `polygon.c` to build different shapes or meshes.
  * Use `polynomial.c` for parametric curves or easing functions.
  * Drive geometry using numerical routines from the math library (e.g., spline paths, Chebyshev approximations).

* **Add gameplay**

  * Build a game-specific state machine in `main.c` or new modules.
  * Drive rendering via `render.c` using the existing WebGL2 pipeline.

* **Improve native support**

  * Introduce a native renderer (SDL2 + OpenGL, Vulkan, etc.).
  * Wire it to the same math / physics core for parity with WASM.

This repo is meant to stay small and understandable while giving you a **solid, reusable spine** for C + WASM experiments in math, physics, and numerical methods.