# -----------------------------------------------------------------------------
# Stage 1: Build WebAssembly artifacts with Emscripten + CMake
# -----------------------------------------------------------------------------
FROM emscripten/emsdk:latest AS builder

# Ninja is used as the generator (matches your usual setup)
RUN apt-get update && apt-get install -y ninja-build && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the project into the image
COPY . .

# Remove any pre-existing build directory (from your host) so we don't reuse
# a CMakeCache.txt that points to /home/jordan/WASM.
RUN rm -rf build-wasm \
 && emcmake cmake -S . -B build-wasm -G Ninja -DCMAKE_BUILD_TYPE=Debug \
 && cmake --build build-wasm

# -----------------------------------------------------------------------------
# Stage 2: Minimal runtime image serving static files via nginx
# -----------------------------------------------------------------------------
FROM nginx:alpine

# Nginx serves static content from /usr/share/nginx/html by default
COPY --from=builder /app/build-wasm/ /usr/share/nginx/html/

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
