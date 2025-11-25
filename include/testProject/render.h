#ifndef RENDER_H
#define RENDER_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
// Define EMSCRIPTEN_KEEPALIVE as empty for native builds
#define EMSCRIPTEN_KEEPALIVE
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Called once at startup to set up GL state & circle vertex buffer
EMSCRIPTEN_KEEPALIVE
int initWebGL(void);

// Begins the main loop that continuously draws the circle
EMSCRIPTEN_KEEPALIVE
void startMainLoop(void);

#ifdef __cplusplus
}
#endif

#endif // RENDER_H