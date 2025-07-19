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

# Detect WSL for browser-opening later
WSL=0
if grep -qi microsoft /proc/version &>/dev/null; then
  WSL=1
fi

# Pick a native generator if Emscripten isn't present
if command -v ninja &>/dev/null; then
  GENERATOR_ARGS=(-G "Ninja")
elif command -v make &>/dev/null; then
  GENERATOR_ARGS=(-G "Unix Makefiles")
else
  echo "❌ Error: neither 'ninja' nor 'make' found."
  exit 1
fi

# Emscripten vs native
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

# Paths for run / serve
NATIVE_EXE="$BUILD_DIR/$PROJECT_NAME"
WASM_JS="$BUILD_DIR/$PROJECT_NAME.js"
INDEX_HTML="$BUILD_DIR/index.html"
PORT=8000
URL="http://localhost:$PORT"

# If native binary exists, just run it
if [[ -x "$NATIVE_EXE" ]]; then
  echo "🚀 Running native: $PROJECT_NAME"
  exec "$NATIVE_EXE" "${EXTRA_ARGS[@]}"
fi

# Otherwise, look for a Wasm build
if [[ -f "$WASM_JS" ]]; then
  echo "🌐 WebAssembly build detected."

  # Ensure an index.html
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
    command -v live-server &>/dev/null || { echo "❌ Install live-server to use --watch"; exit 1; }
    echo "🌍 live-serving with watch → $BUILD_DIR"
    live-server "$BUILD_DIR" --port=$PORT --quiet &
    P=$!
    (( WSL )) && cmd.exe /C start "" "$URL" || xdg-open "$URL" || open "$URL" || true
    trap "kill $P; exit" SIGINT
    # (file-watch rebuild loop can go here if you like…)
    wait $P
    exit 0
  fi

  # No-serve mode
  if [[ $NO_SERVE -eq 1 ]]; then
    echo "✅ Build complete (no-serve)"
    exit 0
  fi

  # ―――――――――――――――――――――――――――――――――――
  # Default serve (smart fallback chain)
  echo "🚀 Serving → $URL"
  if command -v live-server >/dev/null 2>&1; then
    live-server "$BUILD_DIR" --port=$PORT --open=index.html &
  elif command -v http-server >/dev/null 2>&1; then
    http-server "$BUILD_DIR" -p $PORT &
  elif command -v emrun >/dev/null 2>&1; then
    emrun --no_browser --port=$PORT "$INDEX_HTML" &
  elif command -v python3 >/dev/null 2>&1; then
    (cd "$BUILD_DIR" && python3 -m http.server $PORT) &
  elif command -v python >/dev/null 2>&1; then
    (cd "$BUILD_DIR" && python -m SimpleHTTPServer $PORT) &
  else
    echo "🔴 Please install one of: live-server, http-server (npm), or Python 3."
    exit 1
  fi
  P=$!

  # Open browser
  if (( WSL )); then
    cmd.exe /C start "" "$URL"
  else
    xdg-open "$URL" 2>/dev/null || open "$URL" 2>/dev/null || true
  fi

  trap "kill $P; exit" SIGINT
  wait $P
  exit 0

else
  echo "❌ No build output found — looked for '$NATIVE_EXE' or '$WASM_JS'."
  exit 1
fi
