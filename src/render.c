#include "render.h"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

int initWebGL(void) {
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = EM_TRUE;
    attr.depth = EM_TRUE;
    attr.stencil = EM_FALSE;
    attr.antialias = EM_TRUE;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx =
      emscripten_webgl_create_context("#canvas", &attr);
    if (ctx <= 0) return 0;
    emscripten_webgl_make_context_current(ctx);
    return 1;
}

static void tick(void) {
    // simple clear to teal
    glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void startMainLoop(void) {
    emscripten_set_main_loop(tick, 0, EM_TRUE);
}
