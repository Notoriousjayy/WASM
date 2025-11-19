# WASM Native Graphics & Math Engine

A serious, production-ready C foundation for WebAssembly graphics applications with integrated physics simulation, computational geometry, and numerical computing capabilities.

## 🎯 Project Vision

This project serves as a complete, reusable foundation for building:
- **Interactive WebGL2 applications** compiled to WebAssembly
- **Physics simulations** using a Cyclone-inspired engine
- **Numerical computing applications** with TAOCP-grade algorithms
- **Computational geometry demos** with polygon manipulation
- **Cross-platform C libraries** that work in both browser and native contexts

The codebase emphasizes clean architecture, portability, and educational value while maintaining production-grade quality suitable for real applications.

---

## 📋 Table of Contents

- [Features](#-features)
- [Live Demo](#-live-demo)
- [Architecture Overview](#-architecture-overview)
- [Quick Start](#-quick-start)
- [Project Structure](#-project-structure)
- [Module Documentation](#-module-documentation)
- [Build System](#-build-system)
- [CI/CD Pipeline](#-cicd-pipeline)
- [Development Guide](#-development-guide)
- [Extension Points](#-extension-points)
- [Performance Considerations](#-performance-considerations)
- [Roadmap](#-roadmap)

---

## ✨ Features

### WebAssembly & WebGL2
- **Modern WebGL2 rendering** with GLSL 300 ES shaders
- **Efficient vertex buffer management** with dynamic updates
- **Responsive canvas** that auto-resizes to browser window
- **Optimized rendering loop** via Emscripten's main loop API
- **Full C ↔ JavaScript interop** using `ccall` and `cwrap`

### Physics Engine (Cyclone-Inspired)
- **Configurable precision** (float/double) via compile-time switches
- **3D vector, matrix, and quaternion operations** with comprehensive APIs
- **Transform hierarchies** with Matrix3 (rotation/inertia) and Matrix4 (affine transforms)
- **Rigid body math foundations** ready for integration
- **Inline hot-path operations** for maximum performance

### Computational Geometry
- **Dynamic polygon management** with arbitrary vertex counts
- **Geometric transformations**: translate, rotate, scale about origin
- **Regular n-gon generation** for creating any regular polygon
- **Perimeter calculation** for closed polygons
- **WebGL-ready vertex export** to interleaved float arrays

### Polynomial Mathematics
- **Arbitrary-degree polynomials** using sparse linked-list representation
- **Arithmetic operations**: addition, multiplication
- **Polynomial evaluation** at any point
- **Automatic term simplification** and zero removal

### Advanced Linear Algebra
- **2×2, 3×3, 4×4 matrix types** with row-major storage
- **Complete matrix operations**: transpose, inverse, determinant, adjugate
- **Minors and cofactors** for all matrix sizes
- **Euler angle rotations** (X→Y→Z convention)
- **Axis-angle rotations** with proper normalization
- **View and projection matrices** for 3D graphics
- **Orthographic projections** for 2D/UI rendering
- **Column-major conversion** for OpenGL/WebGL compatibility

### Vector Mathematics
- **2D and 3D vector types** with union-based element access
- **Full arithmetic suite**: add, subtract, multiply (component/scalar), divide
- **Dot and cross products** for all applicable dimensions
- **Magnitude and distance calculations** (normal and squared)
- **Normalization** with both in-place and pure functions
- **Angle calculations** between vectors (in radians)
- **Projection and perpendicular** component extraction
- **Reflection** across arbitrary normals
- **2D rotation** by angle in degrees

### Numerical Computing Library
A comprehensive C numerical methods library organized into focused modules:

- **Core Floating-Point** (`mathlib/core/fp.h`): Machine epsilon, ULP, nearly_equal, robust summation
- **Polynomial Operations** (`mathlib/core/poly.h`): Evaluation, basic arithmetic
- **Random Number Generation** (`mathlib/rand/`): PRNGs, uniform/normal distributions, Monte Carlo helpers
- **Interpolation** (`mathlib/interp/`): 1D tables, polynomial, spline, Chebyshev approximation
- **Differentiation** (`mathlib/diffint/deriv.h`): Forward/central differences, Richardson extrapolation
- **Integration** (`mathlib/diffint/quad.h`): Trapezoid, Simpson, Romberg, Gauss-Legendre, adaptive methods
- **Linear Algebra** (`mathlib/linalg/`): Dense matrices, LU/Cholesky decomposition, linear solvers
- **Root Finding** (`mathlib/nonlin/root.h`): Bisection, Newton, Brent, system solvers
- **Optimization** (`mathlib/optim/`): Golden-section, Brent, conjugate gradient, quasi-Newton
- **Spectral Methods** (`mathlib/spectral/fft.h`): FFT/IFFT, spectral derivatives and integrals
- **ODE Solvers** (`mathlib/ode/`): Runge-Kutta, adaptive RK, shooting/relaxation for BVPs
- **PDE Solvers** (`mathlib/pde/`): Finite-difference methods (e.g., 1D heat equation)
- **Statistics** (`mathlib/stats/`): Mean, variance, covariance, least-squares fitting

### Build & Development
- **Single CMake-based toolchain** for both WASM and native builds
- **CMake presets**: `wasm-debug` and `native-debug` for quick configuration
- **Custom HTML shell** with pre-configured WebGL2 module loading
- **Multi-stage Docker build** with Emscripten + nginx
- **Dev server targets** using Python's http.server
- **Comprehensive CI/CD** with GitHub Actions

---

## 🚀 Live Demo

A live hexagon animation demo is automatically deployed to GitHub Pages on every push to `main`:

**[View Live Demo](https://notoriousjayy.github.io/WASM/)** *(replace with your actual GitHub Pages URL)*

The demo features:
- A rotating hexagon with smooth animation
- Orbital motion using parametric equations
- Real-time vertex buffer updates
- Full-screen responsive canvas
- Console logging of geometry calculations

---

## 🏗 Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                         Browser                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │              HTML + JavaScript                        │  │
│  │  • Module.ccall() / cwrap()                           │  │
│  │  • Canvas element management                          │  │
│  │  • Runtime initialization                             │  │
│  └──────────────────┬────────────────────────────────────┘  │
│                     │                                        │
│                     ▼                                        │
│  ┌───────────────────────────────────────────────────────┐  │
│  │           WebAssembly Module (C)                      │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │  Rendering (render.c)                           │  │  │
│  │  │  • WebGL2 context management                    │  │  │
│  │  │  • Shader compilation & linking                 │  │  │
│  │  │  • VAO/VBO setup & updates                      │  │  │
│  │  │  • Main animation loop                          │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │  Geometry (polygon.c, polynomial.c)             │  │  │
│  │  │  • Dynamic polygon generation                   │  │  │
│  │  │  • Geometric transformations                    │  │  │
│  │  │  • Float buffer export                          │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │  Linear Algebra (vectors.c, matrices.c)         │  │  │
│  │  │  • 2D/3D vector operations                      │  │  │
│  │  │  • Matrix arithmetic & transforms               │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │  Physics Core (core.c, precision.c)             │  │  │
│  │  │  • Configurable precision types                 │  │  │
│  │  │  • Cyclone-style physics primitives             │  │  │
│  │  │  • Quaternion & transform math                  │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────────────────────┐  │  │
│  │  │  Numerical Library (mathlib/*)                  │  │  │
│  │  │  • FFT, integration, ODE/PDE solvers            │  │  │
│  │  │  • Optimization & root finding                  │  │  │
│  │  │  • Statistical analysis                         │  │  │
│  │  └─────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### Key Design Principles

1. **Clean Separation of Concerns**
   - Core logic (physics, math) is platform-agnostic
   - Rendering code (`render.c`) is WASM-only and excluded from native builds
   - Headers provide clear API boundaries

2. **Performance-First Design**
   - Inline hot-path operations in headers
   - Row-major matrix storage for cache efficiency
   - Union-based struct member access for flexibility
   - Configurable precision (float/double) at compile time

3. **Portability**
   - C99/C23 standard compliance
   - No platform-specific dependencies in core modules
   - Emscripten-specific code isolated to rendering layer
   - Native builds supported (though rendering is TODO)

4. **Extensibility**
   - Modular architecture with clear interfaces
   - Easy to add new geometry primitives
   - Plugin-style numerical methods library
   - Straightforward to integrate additional physics features

---

## 🚀 Quick Start

### Prerequisites

- **Emscripten SDK** (latest version)
- **CMake** ≥ 3.20
- **Ninja** build system (recommended)
- **Python 3** (for local dev server)
- A modern browser with WebGL2 support

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/notoriousjayy/WASM.git
   cd WASM
   ```

2. **Install Emscripten SDK**
   ```bash
   # Install emsdk
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   cd ..
   ```

3. **Configure the project**
   ```bash
   emcmake cmake --preset wasm-debug
   ```

4. **Build**
   ```bash
   cmake --build --preset wasm-debug -j$(nproc)
   ```

5. **Run locally**
   ```bash
   # Option 1: Using CMake's serve target
   cmake --build --preset wasm-debug --target serve
   
   # Option 2: Using Node dev servers
   npm install
   npx http-server build-wasm -p 8000
   
   # Option 3: Using Python
   cd build-wasm
   python3 -m http.server 8000
   ```

6. **Open in browser**
   - Navigate to `http://localhost:8000`
   - Open DevTools console to see animation logs

### Docker Quick Start

For a completely isolated build environment:

```bash
# Build the Docker image
docker build -t wasm-game .

# Run the container
docker run --rm -p 8080:80 wasm-game
```

Access at `http://localhost:8080`

---

## 📁 Project Structure

```
notoriousjayy-wasm/
├── CMakeLists.txt              # Main build configuration
├── CMakePresets.json           # Build presets (wasm-debug, native-debug)
├── Dockerfile                  # Multi-stage Docker build
├── package.json                # Node.js dev server dependencies
├── readme.md                   # This file
│
├── html_template/
│   └── index.html              # Custom Emscripten HTML shell
│
├── include/testProject/        # Public headers
│   ├── core.h                  # Cyclone physics core (Vector3, Quaternion, Matrix3/4)
│   ├── precision.h             # Configurable float/double precision
│   ├── vectors.h               # 2D/3D vector math
│   ├── matrices.h              # 2×2, 3×3, 4×4 matrix operations
│   ├── polygon.h               # Dynamic polygon management
│   ├── polynomial.h            # Polynomial arithmetic
│   ├── render.h                # WebGL2 rendering interface
│   └── module.h                # Example JS ↔ C interop
│
├── include/mathlib/            # Numerical computing library headers
│   ├── core/                   # Floating-point & polynomial foundations
│   │   ├── fp.h
│   │   └── poly.h
│   ├── rand/                   # Random number generation
│   │   └── rand.h
│   ├── interp/                 # Interpolation & approximation
│   │   └── interp.h
│   ├── diffint/                # Differentiation & integration
│   │   ├── deriv.h
│   │   └── quad.h
│   ├── linalg/                 # Linear algebra
│   │   └── linalg.h
│   ├── nonlin/                 # Nonlinear equations
│   │   └── root.h
│   ├── optim/                  # Optimization
│   │   └── optim.h
│   ├── spectral/               # FFT & spectral methods
│   │   └── fft.h
│   ├── ode/                    # ODE solvers
│   │   ├── ivp.h
│   │   └── bvp.h
│   ├── pde/                    # PDE solvers
│   │   └── pde.h
│   └── stats/                  # Statistics
│       └── stats.h
│
├── src/                        # Implementation files
│   ├── main.c                  # Entry point
│   ├── core.c                  # Physics core implementation
│   ├── precision.c             # Precision helpers
│   ├── vectors.c               # Vector operations
│   ├── matrices.c              # Matrix operations
│   ├── polygon.c               # Polygon management
│   ├── polynomial.c            # Polynomial operations
│   ├── render.c                # WebGL2 renderer (WASM-only)
│   ├── module.c                # Example exported function
│   └── mathlib/                # Numerical library implementations
│       ├── core/               # (fp.c, poly.c)
│       ├── rand/               # (rand.c)
│       ├── interp/             # (interp.c)
│       ├── diffint/            # (deriv.c, quad.c)
│       ├── linalg/             # (linalg.c)
│       ├── nonlin/             # (root.c)
│       ├── optim/              # (optim.c)
│       ├── spectral/           # (fft.c)
│       ├── ode/                # (ivp.c, bvp.c)
│       ├── pde/                # (pde.c)
│       └── stats/              # (stats.c)
│
└── .github/workflows/          # CI/CD pipelines
    ├── build-wasm.yml          # Build & artifact upload
    ├── pages.yml               # GitHub Pages deployment
    ├── release.yml             # Tagged release builds
    ├── codeql.yml              # Security analysis
    └── dependency-review.yml   # Dependency scanning
```

---

## 📚 Module Documentation

### Precision System (`precision.h`, `precision.c`)

Controls floating-point precision throughout the physics engine.

**Configuration:**
```c
// In CMakeLists.txt or via compiler flag:
#define CYCLONE_USE_SINGLE_PRECISION 1  // Use float
#define CYCLONE_USE_SINGLE_PRECISION 0  // Use double (default)
```

**API:**
```c
typedef real;                    // float or double
#define real_sqrt(x)            // sqrtf or sqrt
#define real_sin(x)             // sinf or sin
#define real_cos(x)             // cosf or cos
#define real_epsilon            // FLT_EPSILON or DBL_EPSILON
#define R_PI                    // Pi at chosen precision

real real_deg_to_rad(real degrees);
real real_rad_to_deg(real radians);
```

### Core Physics (`core.h`, `core.c`)

Cyclone-style physics primitives for 3D simulation.

**Vector3 Operations:**
```c
cyclone_Vector3 v = cyclone_vector3_make(1.0, 2.0, 3.0);
cyclone_vector3_normalize(&v);
real mag = cyclone_vector3_magnitude(&v);
cyclone_Vector3 cross = cyclone_vector3_cross(v1, v2);
real dot = cyclone_vector3_dot(v1, v2);
```

**Quaternion Operations:**
```c
cyclone_Quaternion q = cyclone_quaternion_identity();
cyclone_quaternion_normalise(&q);
cyclone_quaternion_rotate_by_vector(&q, &angular_velocity);
```

**Matrix4 (3×4 Transform):**
```c
cyclone_Matrix4 m = cyclone_matrix4_identity();
cyclone_matrix4_set_orientation_and_pos(&m, &quat, &pos);
cyclone_Vector3 transformed = cyclone_matrix4_transform(&m, &v);
real det = cyclone_matrix4_get_determinant(&m);
cyclone_Matrix4 inv = cyclone_matrix4_inverse(&m);
```

**Matrix3 (Rotation/Inertia):**
```c
cyclone_Matrix3 m3 = cyclone_matrix3_zero();
cyclone_matrix3_set_orientation(&m3, &quat);
cyclone_matrix3_set_block_inertia_tensor(&m3, &half_sizes, mass);
cyclone_Matrix3 inv = cyclone_matrix3_inverse(&m3);
```

### Vectors (`vectors.h`, `vectors.c`)

2D and 3D vector operations for graphics and gameplay.

**Construction:**
```c
vec2 v2 = vec2_make(1.0f, 2.0f);
vec3 v3 = vec3_make(1.0f, 2.0f, 3.0f);
```

**Arithmetic:**
```c
vec3 sum = vec3_add(a, b);
vec3 diff = vec3_sub(a, b);
vec3 scaled = vec3_mul_scalar(v, 2.5f);
vec3 component_product = vec3_mul(a, b);
```

**Geometric Operations:**
```c
float len = vec3_magnitude(v);
vec3 unit = vec3_normalized(v);
float angle = vec3_angle(a, b);  // radians
vec3 proj = vec3_project(length, direction);
vec3 perp = vec3_perpendicular(length, direction);
vec3 reflected = vec3_reflect(incoming, normal);
vec3 cross = vec3_cross(a, b);  // 3D only
```

### Matrices (`matrices.h`, `matrices.c`)

Comprehensive matrix operations for transforms and projections.

**Basic Operations:**
```c
mat4 m = mat4_identity();
mat4 t = mat4_transpose(m);
mat4 inv = mat4_inverse(m);
float det = mat4_determinant(m);
mat4 product = mat4_mul(m1, m2);
```

**Transforms:**
```c
mat4 translation = mat4_translation_xyz(10.0f, 0.0f, 5.0f);
mat4 scale = mat4_scale_vec3(vec3_make(2.0f, 2.0f, 2.0f));
mat4 rotation = Rotation(pitch, yaw, roll);  // Euler angles
mat4 axis_rot = AxisAngle(axis, angle);      // Axis-angle

// Composite transform: Scale → Rotate → Translate
mat4 transform = TransformEuler(scale_vec, euler_rot, translate_vec);
```

**Camera & Projection:**
```c
mat4 view = LookAt(eye_pos, target, up);
mat4 proj = Projection(fov, aspect, near, far);  // Perspective
mat4 ortho = Ortho(left, right, bottom, top, near, far);  // Orthographic
```

**Vector Transformation:**
```c
vec3 point = MultiplyPoint(v, transform);      // w=1 (position)
vec3 direction = mat4_multiply_vector(v, m);   // w=0 (direction)
```

### Polygons (`polygon.h`, `polygon.c`)

Dynamic polygon management for geometry manipulation.

**Creation & Manipulation:**
```c
Polygon poly;
polygon_init(&poly);

// Create regular hexagon with radius 1.0
polygon_make_regular_ngon(&poly, 6, 1.0);

// Manual vertex addition
polygon_add_vertex(&poly, 0.5, 0.5);
polygon_insert_vertex(&poly, 2, 1.0, 0.0);
polygon_remove_vertex(&poly, 3);
```

**Transformations:**
```c
polygon_translate(&poly, dx, dy);
polygon_scale(&poly, sx, sy);
polygon_rotate(&poly, radians);
```

**Queries:**
```c
bool valid = polygon_is_valid(&poly);  // Has ≥3 vertices
double perim = polygon_perimeter(&poly);
```

**WebGL Export:**
```c
// Option 1: Copy to existing buffer
float vertex_buffer[100];
size_t count = polygon_copy_to_float_xy(&poly, vertex_buffer, 100);

// Option 2: Allocate new buffer (caller must free)
size_t float_count;
float *verts = polygon_alloc_float_xy(&poly, &float_count);
// ... use verts ...
free(verts);
```

### Polynomials (`polynomial.h`, `polynomial.c`)

Sparse polynomial representation using linked lists.

**Usage:**
```c
Polynomial p, q, sum, product;
polynomial_init(&p);
polynomial_init(&q);

// Build p(x) = 3x² + 2x + 1
polynomial_insert_term(&p, 3.0, 2);
polynomial_insert_term(&p, 2.0, 1);
polynomial_insert_term(&p, 1.0, 0);

// Evaluate
double result = polynomial_evaluate(&p, 2.0);  // p(2)

// Arithmetic
polynomial_add(&p, &q, &sum);
polynomial_multiply(&p, &q, &product);

// Degree
int deg = polynomial_degree(&p);

// Cleanup
polynomial_clear(&p);
```

### Rendering (`render.h`, `render.c`)

WebGL2-specific rendering layer (WASM builds only).

**Initialization Flow:**
```c
// Called from JavaScript (see html_template/index.html):
int success = initWebGL();  // Setup WebGL2, shaders, VAO/VBO
if (success) {
    startMainLoop();  // Begin animation loop
}
```

**Current Demo:**
- Creates a regular hexagon using `polygon_make_regular_ngon()`
- Animates with rotation and orbital motion
- Updates vertex buffer every frame via `glBufferSubData()`
- Draws with `GL_LINE_LOOP` for wireframe rendering
- Logs animation state to console periodically

**Extension Points:**
- Add color/texture uniform support
- Implement filled polygons (`GL_TRIANGLE_FAN`)
- Add multiple objects with transform hierarchies
- Integrate physics simulation for motion

### Numerical Computing Library (`mathlib/*`)

A comprehensive numerical methods library inspired by *The Art of Computer Programming* Vol. 2 and *Numerical Recipes*.

**Module Overview:**

| Module | Purpose | Key Functions |
|--------|---------|---------------|
| `fp.h` | Floating-point utils | `machine_epsilon()`, `ulp()`, `nearly_equal()`, `robust_sum()` |
| `poly.h` | Polynomial ops | `poly_eval()`, `poly_add()`, `poly_mul()` |
| `rand.h` | Random numbers | `rand_uniform()`, `rand_normal()`, `monte_carlo_integrate()` |
| `interp.h` | Interpolation | `interp_table()`, `poly_interp()`, `spline()`, `chebyshev_approx()` |
| `deriv.h` | Differentiation | `forward_diff()`, `central_diff()`, `richardson()` |
| `quad.h` | Integration | `trapezoid()`, `simpson()`, `romberg()`, `gauss_legendre()`, `adaptive()` |
| `linalg.h` | Linear algebra | `lu_decompose()`, `cholesky()`, `linear_solve()` |
| `root.h` | Root finding | `bisection()`, `newton()`, `brent()`, `system_solve()` |
| `optim.h` | Optimization | `golden_section()`, `brent_minimize()`, `conjugate_gradient()` |
| `fft.h` | Spectral methods | `fft()`, `ifft()`, `spectral_derivative()` |
| `ivp.h` | ODE (initial value) | `rk4()`, `adaptive_rk()` |
| `bvp.h` | ODE (boundary) | `shooting()`, `relaxation()` |
| `pde.h` | PDE solvers | `heat_equation_1d()` |
| `stats.h` | Statistics | `mean()`, `variance()`, `least_squares()` |

**Design Philosophy:**
- **Modular**: Each domain is self-contained
- **Reusable**: Link only what you need
- **Portable**: Pure C, no external dependencies
- **Educational**: Clear implementations of canonical algorithms
- **Production-Ready**: Error handling, numerical stability considerations

---

## 🔧 Build System

### CMake Configuration

**Core Settings:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(testProject C)
set(CMAKE_C_STANDARD 99)
```

**Build Presets:**

```bash
# WebAssembly (Debug)
emcmake cmake --preset wasm-debug
cmake --build --preset wasm-debug

# Native (Debug)
cmake --preset native-debug
cmake --build --preset native-debug
```

**Emscripten-Specific Options:**

The build system automatically configures:
- WebGL2 bindings (`-sUSE_WEBGL2=1`)
- Memory growth (`-sALLOW_MEMORY_GROWTH=1`)
- Exported functions (`_initWebGL`, `_startMainLoop`, `_myFunction`)
- Runtime methods (`ccall`, `cwrap`)
- Custom HTML shell (if present)
- Output as `index.html/js/wasm` for easy serving

**Native Build Notes:**

Native builds exclude `render.c` since WebGL2 is browser-only. To add native rendering:
1. Create `render_native.c` with OpenGL/SDL2/GLFW implementation
2. Update `CMakeLists.txt` to include it for native builds
3. Link appropriate graphics libraries

### Dev Server

Three ways to serve locally:

```bash
# 1. CMake target (Python-based)
cmake --build --preset wasm-debug --target serve

# 2. Node.js http-server
npm install
npx http-server build-wasm -p 8000

# 3. Python directly
cd build-wasm && python3 -m http.server 8000
```

All methods serve on `http://localhost:8000` by default. Ports are configurable via CMake variables:

```bash
cmake --preset wasm-debug -DSERVE_PORT=5173 -DSERVE_HOST=127.0.0.1
```

---

## 🔄 CI/CD Pipeline

### Automated Workflows

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| **build-wasm.yml** | Push/PR to `main` | Build validation, artifact upload |
| **pages.yml** | Push to `main` | Deploy live demo to GitHub Pages |
| **release.yml** | Tag `v*.*.*` | Create GitHub Release with `pong-wasm.zip` |
| **codeql.yml** | Push/PR/Weekly | Security analysis via CodeQL |
| **dependency-review.yml** | PR opened/updated | Scan for vulnerable dependencies |

### GitHub Pages Deployment

**Automatic deployment on every push to `main`:**

1. Build with Emscripten
2. Copy `index.html`, `index.js`, `index.wasm` to `site/`
3. Deploy via `actions/deploy-pages`
4. Live at `https://<username>.github.io/<repo>/`

### Release Process

**Creating a release:**

```bash
git tag v1.0.0
git push origin v1.0.0
```

This triggers:
1. Full WASM build
2. Package as `pong-wasm.zip` with all assets
3. Create GitHub Release with zip attachment
4. Release notes auto-generated

### Caching Strategy

Emscripten's build cache is preserved across runs for faster CI:

```yaml
- uses: actions/cache@v4
  with:
    path: |
      ~/.emscripten_cache
      /tmp/emsdk/upstream/emscripten/cache
    key: emsdk-${{ runner.os }}-latest
```

---

## 💻 Development Guide

### Project Setup

1. **Fork and clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/WASM.git
   cd WASM
   ```

2. **Install dependencies**
   - Install Emscripten SDK (see Quick Start)
   - Install CMake ≥3.20
   - Install Ninja: `sudo apt install ninja-build` (Linux) or `brew install ninja` (macOS)

3. **Configure for development**
   ```bash
   source /path/to/emsdk/emsdk_env.sh
   emcmake cmake --preset wasm-debug
   ```

4. **Iterative development**
   ```bash
   # Make changes to C source
   vim src/render.c
   
   # Rebuild (incremental)
   cmake --build --preset wasm-debug
   
   # Serve and test
   cmake --build --preset wasm-debug --target serve
   # Open http://localhost:8000 in browser
   ```

### Code Organization Best Practices

**Adding New Geometry Primitives:**

1. **Define interface** in `include/testProject/your_shape.h`:
   ```c
   typedef struct YourShape {
       Point2D *vertices;
       size_t count;
       // ... additional fields
   } YourShape;
   
   void your_shape_init(YourShape *shape);
   bool your_shape_generate(YourShape *shape, /* params */);
   void your_shape_clear(YourShape *shape);
   ```

2. **Implement** in `src/your_shape.c`:
   ```c
   #include "your_shape.h"
   #include <stdlib.h>
   
   void your_shape_init(YourShape *shape) {
       shape->vertices = NULL;
       shape->count = 0;
   }
   // ... implement remaining functions
   ```

3. **Update CMakeLists.txt** if needed (usually automatic with `GLOB`)

4. **Use in render.c**:
   ```c
   #include "your_shape.h"
   
   static YourShape g_shape;
   
   // In initWebGL():
   your_shape_generate(&g_shape, /* params */);
   
   // In tick():
   // Update and render your shape
   ```

**Adding Math Library Modules:**

1. **Create header** in `include/mathlib/newmodule/newmodule.h`
2. **Implement** in `src/mathlib/newmodule/newmodule.c`
3. **Follow module pattern**: clear function naming, error handling, documentation
4. **Link selectively**: Only include modules you need in builds

**WebGL Integration Pattern:**

```c
// 1. Define global state
static GLuint g_program;
static GLuint g_vao;
static GLuint g_vbo;
static float *g_vertex_data;
static size_t g_vertex_count;

// 2. Initialize once in initWebGL()
void setup_geometry(void) {
    // Generate data
    // Create VAO/VBO
    // Upload initial data
}

void compile_shaders(void) {
    // Create program
    // Compile vertex/fragment shaders
    // Link program
}

// 3. Update per-frame in tick()
void update_geometry(void) {
    // Modify g_vertex_data
    // Upload with glBufferSubData()
}

void render(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_program);
    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, g_vertex_count);
}
```

### Debugging Tips

**Browser Console:**
```javascript
// Access exported functions
Module._myFunction();

// Check canvas state
const canvas = document.getElementById('canvas');
console.log(canvas.width, canvas.height);

// Monitor WebGL errors
const gl = canvas.getContext('webgl2');
console.log(gl.getError());
```

**Emscripten Debug Flags:**
```cmake
# Add to CMakeLists.txt for debugging:
target_link_options(testProject PRIVATE
    "SHELL:-sASSERTIONS=1"              # Runtime checks
    "SHELL:-sSAFE_HEAP=1"               # Memory access validation
    "SHELL:-sSTACK_OVERFLOW_CHECK=2"    # Stack monitoring
    "-gsource-map"                       # Source map generation
)
```

**Print Debugging:**
```c
#include <stdio.h>

// These appear in browser console
printf("Debug: value = %f\n", value);
fprintf(stderr, "Error: failed at line %d\n", __LINE__);

// Use EM_ASM for JavaScript interop
#include <emscripten.h>
EM_ASM({
    console.log('From C:', $0);
}, value);
```

**Common Issues:**

| Problem | Solution |
|---------|----------|
| Black canvas | Check `glGetError()`, verify shader compilation |
| Artifacts missing | Ensure `emcmake` is used for configure step |
| Crash on load | Check exported functions list, verify WASM loads |
| Performance issues | Profile with browser DevTools, reduce draw calls |
| Memory errors | Enable `SAFE_HEAP`, check buffer bounds |

### Testing Strategy

**Unit Testing (Native Builds):**

```c
// test_vectors.c
#include "vectors.h"
#include <assert.h>
#include <math.h>

void test_vec3_magnitude(void) {
    vec3 v = vec3_make(3.0f, 4.0f, 0.0f);
    float mag = vec3_magnitude(v);
    assert(fabsf(mag - 5.0f) < 1e-6f);
}

int main(void) {
    test_vec3_magnitude();
    // ... more tests
    printf("All tests passed!\n");
    return 0;
}
```

Compile natively:
```bash
cmake --preset native-debug
cmake --build --preset native-debug
./build-native/test_vectors
```

**Integration Testing (WASM):**

Use browser automation (Playwright/Puppeteer):
```javascript
// test.js
const { chromium } = require('playwright');

(async () => {
  const browser = await chromium.launch();
  const page = await browser.newPage();
  
  await page.goto('http://localhost:8000');
  
  // Wait for WASM to load
  await page.waitForFunction(() => typeof Module !== 'undefined');
  
  // Call exported function
  const result = await page.evaluate(() => {
    return Module.ccall('myFunction', 'number', [], []);
  });
  
  console.log('Test result:', result);
  await browser.close();
})();
```

---

## 🔌 Extension Points

### Adding Physics Simulation

The Cyclone-style core provides foundations for rigid body dynamics:

```c
// Example: Simple particle system
typedef struct Particle {
    cyclone_Vector3 position;
    cyclone_Vector3 velocity;
    cyclone_Vector3 acceleration;
    real inverse_mass;
} Particle;

void particle_integrate(Particle *p, real duration) {
    if (p->inverse_mass <= 0.0) return;  // Infinite mass
    
    // Update position
    cyclone_vector3_add_scaled(&p->position, &p->velocity, duration);
    
    // Update velocity from acceleration
    cyclone_Vector3 resulting_acc = p->acceleration;
    cyclone_vector3_add_scaled(&p->velocity, &resulting_acc, duration);
    
    // Apply damping
    cyclone_vector3_scale_inplace(&p->velocity, real_pow(0.99, duration));
}
```

**Integration with Rendering:**

```c
// In render.c tick():
static Particle g_particle;

void tick(void) {
    // Physics update
    particle_integrate(&g_particle, delta_time);
    
    // Sync to polygon position
    g_polygon.vertices[0].x = (float)g_particle.position.x;
    g_polygon.vertices[0].y = (float)g_particle.position.y;
    
    // Render
    // ...
}
```

### Shader Customization

**Adding Uniforms:**

```c
// Vertex shader with MVP matrix
static const char *VERT_SRC =
    "#version 300 es\n"
    "layout(location = 0) in vec2 aPos;\n"
    "uniform mat4 uMVP;\n"
    "void main() {\n"
    "  gl_Position = uMVP * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

// In initWebGL():
GLint mvp_loc = glGetUniformLocation(program, "uMVP");

// In tick():
mat4 mvp = mat4_identity();  // Compute your transform
glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, mvp.asArray);
```

**Fragment Shader Effects:**

```c
// Gradient effect
static const char *FRAG_SRC =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 vTexCoord;\n"
    "out vec4 outColor;\n"
    "uniform float uTime;\n"
    "void main() {\n"
    "  float r = 0.5 + 0.5 * sin(uTime + vTexCoord.x * 3.14);\n"
    "  float g = 0.5 + 0.5 * cos(uTime + vTexCoord.y * 3.14);\n"
    "  outColor = vec4(r, g, 0.5, 1.0);\n"
    "}\n";
```

### Numerical Computing Examples

**Solving ODEs:**

```c
#include "mathlib/ode/ivp.h"

// dy/dt = -k*y (exponential decay)
void decay_function(double t, const double *y, double *dydt, void *params) {
    double k = *(double *)params;
    dydt[0] = -k * y[0];
}

void simulate_decay(void) {
    double y[1] = { 1.0 };  // Initial condition
    double t = 0.0;
    double dt = 0.1;
    double k = 0.5;
    
    for (int i = 0; i < 100; i++) {
        rk4_step(decay_function, t, y, 1, dt, &k);
        t += dt;
        printf("t=%f, y=%f\n", t, y[0]);
    }
}
```

**FFT-Based Filtering:**

```c
#include "mathlib/spectral/fft.h"

void lowpass_filter(double *signal, size_t n, double cutoff) {
    // Forward FFT
    double *real = malloc(n * sizeof(double));
    double *imag = malloc(n * sizeof(double));
    fft(signal, real, imag, n);
    
    // Zero out high frequencies
    for (size_t i = 0; i < n; i++) {
        double freq = (double)i / n;
        if (freq > cutoff) {
            real[i] = 0.0;
            imag[i] = 0.0;
        }
    }
    
    // Inverse FFT
    ifft(real, imag, signal, n);
    
    free(real);
    free(imag);
}
```

### Multi-Object Rendering

**Scene Graph Pattern:**

```c
typedef struct RenderObject {
    Polygon geometry;
    mat4 transform;
    vec3 color;
} RenderObject;

typedef struct Scene {
    RenderObject *objects;
    size_t count;
    size_t capacity;
} Scene;

void scene_render(Scene *scene, mat4 view_proj) {
    for (size_t i = 0; i < scene->count; i++) {
        RenderObject *obj = &scene->objects[i];
        
        // Compute MVP
        mat4 mvp = mat4_mul(obj->transform, view_proj);
        
        // Upload uniform
        glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, mvp.asArray);
        glUniform3fv(color_loc, 1, obj->color.asArray);
        
        // Upload geometry and draw
        upload_polygon(&obj->geometry);
        glDrawArrays(GL_TRIANGLE_FAN, 0, obj->geometry.count);
    }
}
```

---

## ⚡ Performance Considerations

### Memory Management

**Stack vs Heap:**
```c
// ✅ Good: Stack allocation for small, fixed-size data
vec3 velocity = vec3_make(1.0f, 0.0f, 0.0f);
mat4 transform = mat4_identity();

// ✅ Good: Heap for dynamic/large data
Polygon *polygons = malloc(1000 * sizeof(Polygon));

// ❌ Avoid: Large stack allocations
// float huge_array[1000000];  // Risk stack overflow
```

**Memory Pools:**
```c
// For frequent allocations/deallocations
typedef struct ParticlePool {
    Particle *particles;
    bool *active;
    size_t capacity;
} ParticlePool;

Particle *pool_allocate(ParticlePool *pool) {
    for (size_t i = 0; i < pool->capacity; i++) {
        if (!pool->active[i]) {
            pool->active[i] = true;
            return &pool->particles[i];
        }
    }
    return NULL;
}
```

### WebGL Optimization

**Batch Rendering:**
```c
// ❌ Avoid: Draw call per object
for (int i = 0; i < 1000; i++) {
    upload_object(i);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// ✅ Good: Single draw call
build_batch_buffer(objects, 1000, batch_buffer);
upload_batch(batch_buffer);
glDrawArrays(GL_TRIANGLES, 0, 6000);
```

**Buffer Updates:**
```c
// ✅ Good: Orphaning for streaming data
glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STREAM_DRAW);  // Orphan
glBufferSubData(GL_ARRAY_BUFFER, 0, size, new_data);        // Upload

// ✅ Good: Mapped buffers for frequent updates
float *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, 
                               GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
memcpy(ptr, new_data, size);
glUnmapBuffer(GL_ARRAY_BUFFER);
```

### Computational Optimization

**SIMD-Friendly Layout:**
```c
// ❌ Avoid: Array of Structures (AoS)
typedef struct {
    vec3 position;
    vec3 velocity;
} Particle;
Particle particles[1000];

// ✅ Good: Structure of Arrays (SoA)
typedef struct {
    float *pos_x, *pos_y, *pos_z;
    float *vel_x, *vel_y, *vel_z;
    size_t count;
} ParticleSystem;
```

**Cache-Friendly Access:**
```c
// ✅ Good: Sequential access pattern
for (size_t i = 0; i < n; i++) {
    result[i] = a[i] + b[i];  // Sequential reads/writes
}

// ❌ Avoid: Random access pattern
for (size_t i = 0; i < n; i++) {
    result[indices[i]] = a[i] + b[i];  // Random writes
}
```

### Profiling

**Browser DevTools:**
- Performance tab → Record → Analyze frame times
- Memory tab → Heap snapshots to detect leaks
- Console → `performance.now()` for timing

**Emscripten Profiling:**
```bash
# Build with profiling
emcmake cmake --preset wasm-debug -DCMAKE_CXX_FLAGS="-profiling"
cmake --build --preset wasm-debug

# Profile in browser (opens detailed report)
```

---

## 🗺 Roadmap

### Short-Term (v1.x)

- [x] Core WebGL2 rendering pipeline
- [x] Polygon geometry system
- [x] Cyclone physics foundations
- [x] Matrix/vector math library
- [x] CI/CD with GitHub Actions
- [ ] **Native renderer** using SDL2 + OpenGL
- [ ] **2D collision detection** (AABB, SAT)
- [ ] **Texture loading** and rendering
- [ ] **Audio playback** via Web Audio API
- [ ] **Input handling** (keyboard, mouse, touch)

### Mid-Term (v2.x)

- [ ] **Complete physics engine**
  - Rigid body dynamics
  - Contact resolution
  - Springs and constraints
- [ ] **3D rendering**
  - Z-buffer and depth testing
  - Perspective-correct texturing
  - Basic lighting (Phong/Blinn-Phong)
- [ ] **Scene graph** with hierarchical transforms
- [ ] **Asset pipeline** (model loading, shader management)
- [ ] **Particle systems** with GPU instancing

### Long-Term (v3.x)

- [ ] **Advanced rendering**
  - Shadow mapping
  - Deferred shading
  - Post-processing effects (bloom, SSAO)
- [ ] **Animation system**
  - Skeletal animation
  - Blend trees
  - Inverse kinematics
- [ ] **Networking** via WebSockets
- [ ] **Scripting layer** (Lua/WASM-embedded)
- [ ] **Tooling**
  - Level editor
  - Visual shader editor
  - Performance profiler

### Research Directions

- **PDE-Based Simulations**: Fluid dynamics, heat transfer
- **Machine Learning Integration**: Neural networks compiled to WASM
- **WebGPU Port**: Migrate from WebGL2 to WebGPU for compute shaders
- **Multi-threading**: Leverage Web Workers and SharedArrayBuffer

---

## 📖 Additional Resources

### Documentation

- **Emscripten**: https://emscripten.org/docs/
- **WebGL2 Fundamentals**: https://webgl2fundamentals.org/
- **CMake**: https://cmake.org/documentation/
- **Cyclone Physics Engine**: *Game Physics Engine Development* by Ian Millington

### Learning Materials

- **Linear Algebra**: *3D Math Primer for Graphics and Game Development*
- **Numerical Methods**: *Numerical Recipes in C*
- **WebAssembly**: https://webassembly.org/getting-started/developers-guide/
- **Game Physics**: *Game Physics Engine Development* (Millington)

### Related Projects

- **emscripten-demos**: Official Emscripten sample projects
- **webgl-samples**: WebGL2 reference implementations
- **sokol**: Minimal cross-platform libraries for C/C++
- **raylib**: Simple gamedev library with WASM support

---

## 🤝 Contributing

Contributions are welcome! Areas of particular interest:

- **Native rendering backend** (SDL2, GLFW)
- **Physics examples** (rigid bodies, soft bodies, fluids)
- **Additional geometry primitives** (Bezier curves, splines)
- **Numerical methods** (add new mathlib modules)
- **Documentation improvements** (tutorials, API docs)
- **Test coverage** (unit tests, integration tests)
- **Performance optimizations**

**Contribution Guidelines:**

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Follow code style**: K&R C style, 4-space indentation
4. **Add tests** where applicable
5. **Update documentation** if APIs change
6. **Commit with clear messages**: `git commit -m "Add Bezier curve primitive"`
7. **Push to your fork**: `git push origin feature/amazing-feature`
8. **Open a Pull Request** with detailed description

---

## 📄 License

This project is distributed under the **MIT License**. See `LICENSE` file for details.

### Third-Party Acknowledgments

- **Emscripten**: University of Illinois/NCSA Open Source License
- **Cyclone Physics Engine**: Concepts from Ian Millington's work
- **Numerical Algorithms**: Inspired by TAOCP and Numerical Recipes

---

## 👤 Author

**NotoriousJay**
- GitHub: [@notoriousjayy](https://github.com/notoriousjayy)
- Project: [WASM Native Graphics Engine](https://github.com/notoriousjayy/WASM)

---

## 🙏 Acknowledgments

Special thanks to:
- The **Emscripten team** for enabling C/C++ in the browser
- **Khronos Group** for WebGL specifications
- **Ian Millington** for Cyclone physics concepts
- The **open-source community** for tools and inspiration

---

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/notoriousjayy/WASM/issues)
- **Discussions**: [GitHub Discussions](https://github.com/notoriousjayy/WASM/discussions)
- **Documentation**: See project wiki (coming soon)

---

## 🔖 Version History

### v0.1.0 (Current)
- Initial release
- WebGL2 rendering with animated hexagon demo
- Complete vector/matrix math library
- Polygon geometry system
- Cyclone physics core foundations
- Numerical computing library structure
- CI/CD pipeline with GitHub Actions
- Docker support

### Planned: v0.2.0
- Native renderer (SDL2)
- Input handling
- 2D collision detection
- Texture support

---

**Built with ❤️ for learning, experimentation, and serious game development.**