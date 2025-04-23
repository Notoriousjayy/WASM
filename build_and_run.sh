#!/usr/bin/env bash
set -euo pipefail

# -----------------------------------------------------------------------------
# build_and_run.sh
#   Configure, build and run a CMake project (native or WebAssembly).
#
#   --no-serve       : just build (no HTTP server / browser open)
#   --watch          : build once, then
#                        • start live-server on $BUILD_DIR (with browser open)
#                        • watch src/ + html_template/ to rebuild on save
# -----------------------------------------------------------------------------

# — Defaults & argument parsing —
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

# — Detect WSL for browser opening —
WSL=0
if grep -qi microsoft /proc/version &>/dev/null; then
  WSL=1
fi

# — Step 1: Pick a native generator (if we fall back) —
if command -v ninja &>/dev/null; then
  GENERATOR_ARGS=(-G "Ninja")
elif command -v make &>/dev/null; then
  GENERATOR_ARGS=(-G "Unix Makefiles")
else
  echo "❌ Error: neither 'ninja' nor 'make' found."
  exit 1
fi

# — Step 2: Detect Emscripten vs native toolchain —
if command -v emcmake &>/dev/null && command -v emmake &>/dev/null; then
  echo "🔧 Detected Emscripten SDK (using emcmake/emmake)"
  CMAKE_CMD=(emcmake cmake)
  BUILD_CMD=(emmake make -C "$BUILD_DIR")
else
  echo "🔧 Using native CMake"
  CMAKE_CMD=(cmake "${GENERATOR_ARGS[@]}")
  BUILD_CMD=(cmake --build "$BUILD_DIR" --config "$BUILD_TYPE")
fi

# — Step 3: Configure & build —
echo "⚙️  Configuring (${BUILD_TYPE}) in '$BUILD_DIR'..."
mkdir -p "$BUILD_DIR"
"${CMAKE_CMD[@]}" -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "🔨 Building..."
"${BUILD_CMD[@]}"

# — Step 4: Locate outputs & HTML shell —
NATIVE_EXE="$BUILD_DIR/$PROJECT_NAME"
WASM_JS="$BUILD_DIR/$PROJECT_NAME.js"
INDEX_HTML="$BUILD_DIR/index.html"
PORT=8000
URL="http://localhost:$PORT"

# — Native path: just run it —
if [[ -x "$NATIVE_EXE" ]]; then
  echo "🚀 Running native executable: $PROJECT_NAME"
  exec "$NATIVE_EXE" "${EXTRA_ARGS[@]}"
fi

# — WebAssembly path: generate or copy your HTML shell —
if [[ -f "$WASM_JS" ]]; then
  echo "🌐 WebAssembly build detected."

  if [[ -f "html_template/index.html" ]]; then
    cp html_template/index.html "$INDEX_HTML"
    echo "📝 Copied custom HTML shell → $INDEX_HTML"
  else
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
      onRuntimeInitialized: function() {
        console.log("🟢 WASM runtime initialized");
      }
    };
  </script>
  <script src="${PROJECT_NAME}.js"></script>
</body>
</html>
HTML
    echo "📝 Generated default HTML shell → $INDEX_HTML"
  fi

  # — WATCH MODE: live-reload + rebuild on save —
  if [[ $WATCH -eq 1 ]]; then
    if ! command -v live-server &>/dev/null; then
      echo "🔴 live-server not found. Install with: npm install -g live-server"
      exit 1
    fi

    echo "🌍 Starting live-server on '$BUILD_DIR' (port $PORT)…"
    # always let live-server manage reload, but we still manually open below
    live-server "$BUILD_DIR" --port=$PORT --quiet &
    LIVESERVER_PID=$!

    # open once
    if (( WSL )); then
      cmd.exe /C start "" "$URL"
    elif command -v xdg-open &>/dev/null; then
      xdg-open "$URL" >/dev/null 2>&1 || true
    elif command -v open &>/dev/null; then
      open "$URL" >/dev/null 2>&1 || true
    fi

    trap 'echo; echo "🛑 Stopping live-server..."; kill $LIVESERVER_PID; exit 0' SIGINT

    echo "🔄 Watching for changes in src/ and html_template/ to rebuild…"
    SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd)/$(basename "$0")"

    if command -v entr &>/dev/null; then
      find src html_template -type f 2>/dev/null | \
        entr -r bash -c "\"$SCRIPT_PATH\" --build-dir \"$BUILD_DIR\" --build-type \"$BUILD_TYPE\" --project-name \"$PROJECT_NAME\" --watch"
    elif command -v inotifywait &>/dev/null; then
      while inotifywait -q -r -e close_write src html_template 2>/dev/null; do
        echo "🔄 Change detected → rebuilding…"
        bash "$SCRIPT_PATH" --build-dir "$BUILD_DIR" --build-type "$BUILD_TYPE" --project-name "$PROJECT_NAME" --watch
      done
    else
      echo "🔴 Neither 'entr' nor 'inotifywait' found; cannot watch files."
      kill $LIVESERVER_PID
      exit 1
    fi

    wait $LIVESERVER_PID
    exit 0
  fi

  # — NO-SERVE: finish after build/copy HTML —
  if [[ $NO_SERVE -eq 1 ]]; then
    echo "✅ Build complete (no-serve mode)."
    exit 0
  fi

  # — DEFAULT SERVE: prefer live-server, else http-server, else error —
  echo "🚀 Serving '$BUILD_DIR' on port $PORT and opening browser…"

  if command -v live-server &>/dev/null; then
    live-server "$BUILD_DIR" --port=$PORT --open=index.html
  elif command -v http-server &>/dev/null; then
    http-server "$BUILD_DIR" -p $PORT &
    SERVER_PID=$!

    # manual open (in case --open isn’t supported or on WSL)
    if (( WSL )); then
      cmd.exe /C start "" "$URL"
    elif command -v xdg-open &>/dev/null; then
      xdg-open "$URL" >/dev/null 2>&1 || true
    elif command -v open &>/dev/null; then
      open "$URL" >/dev/null 2>&1 || true
    fi

    trap 'echo; echo "🛑 Shutting down http-server..."; kill $SERVER_PID; exit 0' SIGINT
    wait $SERVER_PID
  else
    echo "🔴 Neither live-server nor http-server found."
    echo "   Install with: npm install -g live-server http-server"
    exit 1
  fi

else
  echo "❌ Built output not found."
  echo "   Checked for '$NATIVE_EXE' and '$WASM_JS'."
  exit 1
fi
