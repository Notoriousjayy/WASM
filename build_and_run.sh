#!/usr/bin/env bash
set -euo pipefail

# -----------------------------------------------------------------------------
# build_and_run.sh
#   Configure, build and run a CMake project (native or WebAssembly).
#   If a WebAssembly build is detected, copies/generates index.html (with canvas),
#   serves it on localhost:8000, and opens it in your browser.
# -----------------------------------------------------------------------------

# — Defaults & argument parsing —
BUILD_DIR="build"
BUILD_TYPE="Release"
PROJECT_NAME="testProject"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)    BUILD_DIR="$2"; shift 2 ;;
    --build-type)   BUILD_TYPE="$2"; shift 2 ;;
    --project-name) PROJECT_NAME="$2"; shift 2 ;;
    --)             shift; break ;;
    -*) echo "Unknown option: $1"; exit 1 ;;
    *)  break ;;
  esac
done
EXTRA_ARGS=("$@")

# — Step 1: Pick a generator —
if command -v ninja &>/dev/null; then
  GENERATOR_ARGS=(-G "Ninja")
elif command -v make &>/dev/null; then
  GENERATOR_ARGS=(-G "Unix Makefiles")
else
  echo "❌ Error: neither 'ninja' nor 'make' found."
  exit 1
fi

# — Step 2: Configure & build —
echo "⚙️  Configuring (${BUILD_TYPE}) in '$BUILD_DIR'..."
mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" "${GENERATOR_ARGS[@]}" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
echo "🔨 Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

# — Step 3: Decide native vs WASM —
NATIVE_EXE="$BUILD_DIR/$PROJECT_NAME"
WASM_JS="$BUILD_DIR/$PROJECT_NAME.js"
INDEX_HTML="$BUILD_DIR/index.html"
PORT=8000

if [[ -x "$NATIVE_EXE" ]]; then
  echo "🚀 Running native executable: $PROJECT_NAME"
  exec "$NATIVE_EXE" "${EXTRA_ARGS[@]}"

elif [[ -f "$WASM_JS" ]]; then
  echo "🌐 WebAssembly build detected."

  # — Pick up or generate your HTML shell with <canvas> + Module pre-config —
  if [[ -f "html_template/index.html" ]]; then
    cp html_template/index.html "$INDEX_HTML"
    echo "📝 Copied custom HTML shell → $INDEX_HTML"
  else
    cat > "$INDEX_HTML" <<HTML
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>${PROJECT_NAME}</title>
</head>
<body>
  <h1>${PROJECT_NAME}</h1>

  <!-- Your canvas: must match Module.canvas below -->
  <canvas id="canvas" width="800" height="600"></canvas>

  <script>
    // Pre-configure the Module so the glue code picks up your canvas
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

  # — Step 4: Check & free port if needed —
  if command -v lsof &>/dev/null; then
    USED_PIDS=$(lsof -i TCP:"$PORT" -sTCP:LISTEN -t || true)
    if [[ -n "$USED_PIDS" ]]; then
      echo "⚠️  Port $PORT in use by: $USED_PIDS"
      read -rp "Kill them and continue? [y/N] " kill_confirm
      if [[ "$kill_confirm" =~ ^[Yy]$ ]]; then
        kill $USED_PIDS; sleep 1
      else
        echo "Aborted."; exit 1
      fi
    fi
  else
    echo "ℹ️  'lsof' not found—skipping port check."
  fi

  # — Step 5: Serve & open browser —
  pushd "$BUILD_DIR" >/dev/null
    python3 -m http.server "$PORT" &
    SERVER_PID=$!
  popd >/dev/null

  URL="http://localhost:${PORT}/index.html"
  echo "🚀 Serving at: $URL"

  if grep -qi microsoft /proc/version 2>/dev/null; then
    # WSL
    command -v explorer.exe &>/dev/null \
      && explorer.exe "$URL" \
      || (cd ~ && cmd.exe /C start "" "$URL")
  elif command -v xdg-open &>/dev/null; then
    xdg-open "$URL"
  elif command -v open &>/dev/null; then
    open "$URL"
  else
    echo "👀 Please open in your browser: $URL"
  fi

  trap 'echo; echo "🛑 Shutting down server..."; kill $SERVER_PID; exit 0' SIGINT
  wait $SERVER_PID

else
  echo "❌ Built output not found."
  echo "   Checked for '$NATIVE_EXE' and '$WASM_JS'."
  exit 1
fi
