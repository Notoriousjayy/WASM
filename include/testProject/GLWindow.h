#ifndef _H_GL_WINDOW_
#define _H_GL_WINDOW_

// GLWindow - OpenGL/WebGL Window Management
// This header provides the rendering context and window interface.
// For WebAssembly builds, this integrates with the HTML5 canvas.

// Forward declarations for window dimensions (if needed by Camera)
// The actual implementation depends on the platform target.

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// Window state structure (extend as needed)
struct GLWindowState {
    int width;
    int height;
    bool isFullscreen;
    bool isVisible;
};

// Function declarations (implement in GLWindow.cpp)
int GetWindowWidth();
int GetWindowHeight();
float GetWindowAspect();

#endif