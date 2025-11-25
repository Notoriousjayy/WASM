# =============================================================================
# Multi-Platform Dockerfile for Web, Mobile, and Desktop
# Supports: WebAssembly, Android, iOS, Windows, Linux, macOS
# =============================================================================

# -----------------------------------------------------------------------------
# Stage: base-deps - Common dependencies for all builds
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS base-deps

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    pkg-config \
    python3 \
    python3-pip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# -----------------------------------------------------------------------------
# Stage: wasm-builder - WebAssembly build with Emscripten
# -----------------------------------------------------------------------------
FROM emscripten/emsdk:latest AS wasm-builder

RUN apt-get update && apt-get install -y ninja-build && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Clean any host build artifacts
RUN rm -rf build-* CMakeCache.txt CMakePresets.json

# Configure and build for WebAssembly
RUN emcmake cmake -B build-wasm-release \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_WASM_HTML=ON \
    -DUSE_SDL3=OFF

RUN cmake --build build-wasm-release --parallel

# Verify artifacts exist
RUN ls -lh build-wasm-release/ && \
    test -f build-wasm-release/index.html && \
    test -f build-wasm-release/index.js && \
    test -f build-wasm-release/index.wasm

# -----------------------------------------------------------------------------
# Stage: linux-builder - Native Linux build with SDL3
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS linux-builder

ENV DEBIAN_FRONTEND=noninteractive

# Install SDL3 dependencies and build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    libgl1-mesa-dev \
    libgles2-mesa-dev \
    libx11-dev \
    libxext-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxss-dev \
    libxxf86vm-dev \
    libxtst-dev \
    libxkbcommon-dev \
    libwayland-dev \
    wayland-protocols \
    libdbus-1-dev \
    libudev-dev \
    libasound2-dev \
    libpulse-dev \
    libpipewire-0.3-dev \
    libjack-dev \
    libsndio-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Clone and build SDL3
RUN git clone --depth 1 --branch main https://github.com/libsdl-org/SDL.git /tmp/SDL && \
    cmake -B /tmp/SDL/build -S /tmp/SDL \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DSDL_TEST=OFF \
        -DSDL_STATIC=OFF \
        -DSDL_SHARED=ON && \
    cmake --build /tmp/SDL/build --parallel && \
    cmake --install /tmp/SDL/build && \
    rm -rf /tmp/SDL

COPY . .

RUN rm -rf build-* CMakeCache.txt

# Build native Linux binary with SDL3
RUN cmake -B build-native-release \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SDL3=ON && \
    cmake --build build-native-release --parallel

# Verify binary exists
RUN ls -lh build-native-release/ && \
    test -f build-native-release/testProject

# -----------------------------------------------------------------------------
# Stage: android-builder - Android build
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS android-builder

ENV DEBIAN_FRONTEND=noninteractive
ENV ANDROID_SDK_ROOT=/opt/android-sdk
ENV ANDROID_NDK_VERSION=26.1.10909125
ENV ANDROID_NDK_ROOT=${ANDROID_SDK_ROOT}/ndk/${ANDROID_NDK_VERSION}

# Install Android build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    openjdk-17-jdk \
    wget \
    unzip \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install Android SDK Command Line Tools
RUN mkdir -p ${ANDROID_SDK_ROOT}/cmdline-tools && \
    cd ${ANDROID_SDK_ROOT}/cmdline-tools && \
    wget -q https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip && \
    unzip -q commandlinetools-linux-9477386_latest.zip && \
    rm commandlinetools-linux-9477386_latest.zip && \
    mv cmdline-tools latest

ENV PATH="${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin:${PATH}"

# Accept licenses and install NDK
RUN yes | sdkmanager --licenses && \
    sdkmanager "ndk;${ANDROID_NDK_VERSION}" "platforms;android-33" "build-tools;33.0.2"

WORKDIR /app

# Clone and build SDL3 for Android
RUN git clone --depth 1 --branch main https://github.com/libsdl-org/SDL.git /tmp/SDL && \
    cmake -B /tmp/SDL/build-android -S /tmp/SDL \
        -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM=android-24 \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/android-sdl3 && \
    cmake --build /tmp/SDL/build-android --parallel && \
    cmake --install /tmp/SDL/build-android && \
    rm -rf /tmp/SDL

COPY . .

RUN rm -rf build-* CMakeCache.txt

# Configure for Android (ARM64-v8a)
RUN cmake -B build-android \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SDL3=ON \
    -DSDL3_DIR=/opt/android-sdl3/lib/cmake/SDL3

RUN cmake --build build-android --parallel

# Verify library exists
RUN ls -lh build-android/ && \
    find build-android -name "*.so" -type f

# -----------------------------------------------------------------------------
# Stage: windows-builder - Windows cross-compilation (MinGW)
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS windows-builder

ENV DEBIAN_FRONTEND=noninteractive

# Install MinGW cross-compilation toolchain
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    mingw-w64 \
    wine64 \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN rm -rf build-* CMakeCache.txt

# Create toolchain file for MinGW
RUN mkdir -p .docker/toolchains && \
    cat > .docker/toolchains/mingw-w64-x86_64.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

# Cross-compile for Windows (without SDL3 for now)
RUN cmake -B build-windows \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=.docker/toolchains/mingw-w64-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SDL3=OFF && \
    cmake --build build-windows --parallel || echo "Windows build attempted"

# -----------------------------------------------------------------------------
# Stage: artifacts-collector - Collect all build artifacts
# -----------------------------------------------------------------------------
FROM alpine:latest AS artifacts-collector

WORKDIR /artifacts

# Copy WebAssembly artifacts
COPY --from=wasm-builder /app/build-wasm-release/ ./wasm/

# Copy Linux artifacts
COPY --from=linux-builder /app/build-native-release/ ./linux/

# Copy Android artifacts
COPY --from=android-builder /app/build-android/ ./android/

# Copy Windows artifacts (if successful)
COPY --from=windows-builder /app/build-windows/ ./windows/ || echo "Windows artifacts not available"

# Create manifest
RUN echo "Build artifacts:" > manifest.txt && \
    echo "================" >> manifest.txt && \
    echo "" >> manifest.txt && \
    echo "WebAssembly:" >> manifest.txt && \
    ls -lh wasm/ >> manifest.txt && \
    echo "" >> manifest.txt && \
    echo "Linux:" >> manifest.txt && \
    ls -lh linux/ >> manifest.txt && \
    echo "" >> manifest.txt && \
    echo "Android:" >> manifest.txt && \
    ls -lh android/ >> manifest.txt && \
    cat manifest.txt

# -----------------------------------------------------------------------------
# Stage: wasm-server - Production WebAssembly server
# -----------------------------------------------------------------------------
FROM nginx:alpine AS wasm-server

# Copy only WebAssembly artifacts
COPY --from=wasm-builder /app/build-wasm-release/ /usr/share/nginx/html/

# Create proper nginx configuration with correct syntax
RUN cat > /etc/nginx/conf.d/default.conf << 'EOF'
server {
    listen 80;
    server_name localhost;
    root /usr/share/nginx/html;
    index index.html index.htm;

    # Enable CORS headers
    add_header 'Access-Control-Allow-Origin' '*' always;
    add_header 'Cross-Origin-Opener-Policy' 'same-origin' always;
    add_header 'Cross-Origin-Embedder-Policy' 'require-corp' always;

    # Correct MIME types for WebAssembly
    types {
        text/html                             html htm shtml;
        text/css                              css;
        application/javascript                js;
        application/wasm                      wasm;
        image/png                             png;
        image/jpeg                            jpeg jpg;
    }

    location / {
        try_files $uri $uri/ /index.html;
    }

    # Specific handling for .wasm files
    location ~* \.wasm$ {
        add_header 'Content-Type' 'application/wasm';
        add_header 'Access-Control-Allow-Origin' '*';
    }
}
EOF

# Verify the config is valid
RUN nginx -t

EXPOSE 80

STOPSIGNAL SIGTERM

CMD ["nginx", "-g", "daemon off;"]

# -----------------------------------------------------------------------------
# Stage: linux-runtime - Production Linux runtime
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS linux-runtime

ENV DEBIAN_FRONTEND=noninteractive

# Install only runtime dependencies (much smaller than build deps)
RUN apt-get update && apt-get install -y \
    libgl1 \
    libopengl0 \
    libgles2 \
    libx11-6 \
    libxext6 \
    libxcursor1 \
    libxi6 \
    libxinerama1 \
    libxrandr2 \
    libxss1 \
    libxxf86vm1 \
    libxtst6 \
    libxkbcommon0 \
    libwayland-client0 \
    libwayland-egl1 \
    libdbus-1-3 \
    libudev1 \
    libasound2 \
    libpulse0 \
    libpipewire-0.3-0 \
    libjack0 \
    libsndio7.0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the binary and required libraries
COPY --from=linux-builder /app/build-native-release/testProject /app/testProject
COPY --from=linux-builder /usr/local/lib/libSDL3.so* /usr/local/lib/

# Update library cache
RUN ldconfig

# Make executable
RUN chmod +x /app/testProject

EXPOSE 8080

CMD ["/app/testProject"]

# -----------------------------------------------------------------------------
# Stage: dev-environment - Development environment with all tools
# -----------------------------------------------------------------------------
FROM ubuntu:22.04 AS dev-environment

ENV DEBIAN_FRONTEND=noninteractive

# Install ALL build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    pkg-config \
    python3 \
    python3-pip \
    gdb \
    valgrind \
    clang \
    clang-format \
    clang-tidy \
    libgl1-mesa-dev \
    libgles2-mesa-dev \
    libx11-dev \
    libxext-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxss-dev \
    libxxf86vm-dev \
    libxtst-dev \
    libxkbcommon-dev \
    libwayland-dev \
    wayland-protocols \
    libdbus-1-dev \
    libudev-dev \
    libasound2-dev \
    libpulse-dev \
    libpipewire-0.3-dev \
    libjack-dev \
    libsndio-dev \
    vim \
    nano \
    && rm -rf /var/lib/apt/lists/*

# Install Emscripten
RUN git clone https://github.com/emscripten-core/emsdk.git /opt/emsdk && \
    cd /opt/emsdk && \
    ./emsdk install latest && \
    ./emsdk activate latest

ENV EMSDK=/opt/emsdk
ENV PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${PATH}"

WORKDIR /workspace

# Keep container running for development
CMD ["/bin/bash"]