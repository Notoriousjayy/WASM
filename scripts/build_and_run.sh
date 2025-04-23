#!/usr/bin/env bash
set -euo pipefail

# -------------------------------------------------------------------------------
# build_and_run.sh
#   Configure, build and run a CMake project (native or WebAssembly).
# -------------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

BUILD_DIR="build"
BUILD_TYPE="Release"
PROJECT_NAME="testProject"
NO_SERVE=0
WATCH=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)    BUILD_DIR="$2"; shift 2 ;;
    --build-type)   BUILD_TYPE="$2"; shift 2 ;;
    --project-name) PROJECT_NAME="$2"; shift 2 ;;
    --no-serve)     NO_SERVE=1; shift ;;
    --watch)        WATCH=1; shift ;;
    --)             shift; break ;;
    -*) echo "Unknown option: $1"; exit 1 ;;
    *)  break ;;
  esac
done
EXTRA_ARGS=("$@")

# Detect WSL for browser opening
WSL=0
if grep -qi microsoft /proc/version &>/dev/null; then
  WSL=1
fi

# Pick a native generator (if no Emscripten)
if command -v ninja &>/dev/null; then
  GENERATOR_ARGS=(-G "Ninja")
elif command -v make &>/dev/null; then
  GENERATOR_ARGS=(-G "Unix Makefiles")
else
  echo "❌ Error: neither 'ninja' nor 'make' found."
  exit 1
fi

# Detect Emscripten vs native
if command -v emcmake &>/dev/null && command -v emmake &>/dev/null; then
  echo "🔧 Detected Emscripten SDK"
  CMAKE_CMD=(emcmake cmake)
  BUILD_CMD=(emmake make -C "$BUILD_DIR")
else
  echo "🔧 Using native CMake"
  CMAKE_CMD=(cmake "${GENERATOR_ARGS[@]}")
  BUILD_CMD=(cmake --build "$BUILD_DIR" --config "$BUILD_TYPE")
fi

# Configure & build
echo "⚙️  Configuring ${BUILD_TYPE} → $BUILD_DIR"
mkdir -p "$BUILD_DIR"
"${CMAKE_CMD[@]}" -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
echo "🔨 Building..."
"${BUILD_CMD[@]}"

# Locate outputs
NATIVE_EXE="$BUILD_DIR/$PROJECT_NAME"
WASM_JS="$BUILD_DIR/$PROJECT_NAME.js"
INDEX_HTML="$BUILD_DIR/index.html"
PORT=8000
URL="http://localhost:$PORT"

# Native run
if [[ -x "$NATIVE_EXE" ]]; then
  echo "🚀 Running native: $PROJECT_NAME"
  exec "$NATIVE_EXE" "${EXTRA_ARGS[@]}"
fi

# WebAssembly path
if [[ -f "$WASM_JS" ]]; then
  echo "🌐 WebAssembly build detected."

  # Use the HTML CMake already generated
  if [[ -f "$INDEX_HTML" ]]; then
    echo "📝 Using CMake-generated HTML → $INDEX_HTML"
  else
    echo "⚠️  Warning: build/index.html not found; generating minimal shell"
    cat > "$INDEX_HTML" <<HTML
<!DOCTYPE html>
<html lang="en">
<head><meta charset="utf-8"><title>${PROJECT_NAME}</title></head>
<body>
  <h1>${PROJECT_NAME}</h1>
  <canvas id="canvas" width="800" height="600"></canvas>
  <script>
    var Module = {
      canvas: document.getElementById('canvas'),
      onRuntimeInitialized: function(){
        if(!Module.ccall('initWebGL','number',[],[])){
          alert('Failed to init WebGL');
          return;
        }
        Module.ccall('startMainLoop','void',[],[]);
      }
    };
  </script>
  <script src="${PROJECT_NAME}.js"></script>
</body>
</html>
HTML
  fi

  # Watch mode
  if [[ $WATCH -eq 1 ]]; then
    command -v live-server &>/dev/null || { echo "Install live-server"; exit 1; }
    echo "🌍 live-serving with watch → $BUILD_DIR"
    live-server "$BUILD_DIR" --port=$PORT --quiet &
    P=$!
    (( WSL )) && cmd.exe /C start "" "$URL" || xdg-open "$URL" || open "$URL" || true
    trap "kill $P; exit" SIGINT
    # file-watch rebuild loop omitted for brevity…
    wait $P
    exit 0
  fi

  # No-serve mode
  if [[ $NO_SERVE -eq 1 ]]; then
    echo "✅ Build complete (no-serve)"
    exit 0
  fi

  # Default serve
  echo "🚀 Serving → $URL"
  if command -v live-server &>/dev/null; then
    live-server "$BUILD_DIR" --port=$PORT --open=index.html
  elif command -v http-server &>/dev/null; then
    http-server "$BUILD_DIR" -p $PORT &
    P=$!
    (( WSL )) && cmd.exe /C start "" "$URL" || xdg-open "$URL" || open "$URL" || true
    trap "kill $P; exit" SIGINT
    wait $P
  else
    echo "🔴 Install live-server or http-server!"
    exit 1
  fi

else
  echo "❌ No build output — looked for '$NATIVE_EXE' or '$WASM_JS'."
  exit 1
fi
