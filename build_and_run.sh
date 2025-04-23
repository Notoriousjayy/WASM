#!/usr/bin/env bash
set -euo pipefail

# -----------------------------------------------------------------------------
# build_and_run.sh
#
# Configure, build and run a CMake project (native or WebAssembly).
# If a WebAssembly build is detected, generates index.html, serves it on localhost:8000,
# and opens it in your browser (WSL, Linux, or macOS).  Checks for port conflicts.
#
# Usage:
#   ./build_and_run.sh [--build-dir DIR] [--build-type TYPE] [--project-name NAME] [--] [args...]
#
# Examples:
#   ./build_and_run.sh
#   ./build_and_run.sh --build-type Debug --project-name myApp -- --help
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
    --)             shift; break ;;  # remaining args → executable
    -*) echo "Unknown option: $1"; exit 1 ;;
    *)  break ;;
  esac
done

EXTRA_ARGS=("$@")

# — Step 1: Detect build tool (ninja preferred) —
if command -v ninja &>/dev/null; then
  GENERATOR_ARGS=(-G "Ninja")
elif command -v make &>/dev/null; then
  GENERATOR_ARGS=(-G "Unix Makefiles")
else
  echo "❌ Error: neither 'ninja' nor 'make' found in PATH."
  echo "   Install one (e.g. 'sudo apt install ninja-build' or 'build-essential') and retry."
  exit 1
fi

# — Step 2: Configure the project —
echo "⚙️  Configuring (${BUILD_TYPE}) in '$BUILD_DIR' (${GENERATOR_ARGS[*]})..."
mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" \
      "${GENERATOR_ARGS[@]}" \
      -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# — Step 3: Build the project —
echo "🔨 Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"

# — Step 4: Run or serve the result —
NATIVE_EXE="$BUILD_DIR/$PROJECT_NAME"
WASM_JS="$BUILD_DIR/$PROJECT_NAME.js"
INDEX_HTML="$BUILD_DIR/index.html"
PORT=8000

if [[ -x "$NATIVE_EXE" ]]; then
  echo "🚀 Running native executable: $PROJECT_NAME"
  exec "$NATIVE_EXE" "${EXTRA_ARGS[@]}"

elif [[ -f "$WASM_JS" ]]; then
  echo "🌐 WebAssembly build detected."

  # Generate index.html if missing
  if [[ ! -f "$INDEX_HTML" ]]; then
    cat > "$INDEX_HTML" <<HTML
<!DOCTYPE html>
<html>
  <head><meta charset="utf-8"><title>${PROJECT_NAME}</title></head>
  <body>
    <h1>${PROJECT_NAME}</h1>
    <script src="${PROJECT_NAME}.js"></script>
  </body>
</html>
HTML
    echo "📝 Generated: $INDEX_HTML"
  fi

  # — Step 4a: Check port availability —
  if command -v lsof &>/dev/null; then
    USED_PIDS=$(lsof -i TCP:"$PORT" -sTCP:LISTEN -t || true)
    if [[ -n "$USED_PIDS" ]]; then
      echo "⚠️  Port $PORT is already in use by process(es): $USED_PIDS"
      read -rp "Kill them and continue? [y/N] " kill_confirm
      if [[ "$kill_confirm" =~ ^[Yy]$ ]]; then
        echo "🛑 Killing process(es): $USED_PIDS"
        kill $USED_PIDS
        sleep 1
      else
        echo "Aborted by user."
        exit 1
      fi
    fi
  else
    echo "ℹ️  'lsof' not found—skipping port check for $PORT."
  fi

  # Serve via HTTP
  pushd "$BUILD_DIR" >/dev/null
    python3 -m http.server "$PORT" &
    SERVER_PID=$!
  popd >/dev/null

  URL="http://localhost:${PORT}/index.html"
  echo "🚀 Serving at: $URL"

  # Open in browser
  if grep -qi microsoft /proc/version 2>/dev/null; then
    # WSL
    if command -v explorer.exe &>/dev/null; then
      explorer.exe "$URL"
    else
      (cd ~ && cmd.exe /C start "" "$URL")
    fi
  elif command -v xdg-open &>/dev/null; then
    xdg-open "$URL"
  elif command -v open &>/dev/null; then
    open "$URL"
  else
    echo "👀 Please open in your browser: $URL"
  fi

  # Wait until user hits Ctrl+C, then cleanup
  trap 'echo; echo "🛑 Shutting down server..."; kill $SERVER_PID; exit 0' SIGINT
  wait $SERVER_PID

else
  echo "❌ Built output not found."
  echo "   Checked for '$NATIVE_EXE' and '$WASM_JS'."
  exit 1
fi
