/* render.c — polygon renderer with logging + idempotent main loop */

#include "render.h"
#include "polygon.h"

#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Global WebGL objects */
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_ctx = 0;
static GLuint   program     = 0;
static GLuint   vao         = 0;
static GLuint   vbo         = 0;
static GLsizei  vertexCount = 0;
static Polygon  g_polygon;
static bool     g_main_loop_started = false;

/* Simple pass-through vertex shader */
static const char *VERT_SRC =
  "#version 300 es\n"
  "layout(location = 0) in vec2 aPos;\n"
  "void main() {\n"
  "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
  "}\n";

/* Solid-red fragment shader */
static const char *FRAG_SRC =
  "#version 300 es\n"
  "precision mediump float;\n"
  "out vec4 outColor;\n"
  "void main() {\n"
  "  outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
  "}\n";

static GLuint
compileShader(GLenum type, const char *src)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, NULL);
    glCompileShader(sh);

#if 0
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        char *log = (char *)malloc((size_t)len);
        if (log) {
            glGetShaderInfoLog(sh, len, NULL, log);
            fprintf(stderr, "Shader compile error:\n%s\n", log);
            free(log);
        }
    }
#endif
    return sh;
}

/* Resize callback: keep canvas size == browser window size */
static EM_BOOL
resize_callback(int eventType, const EmscriptenUiEvent *e, void *userData)
{
    (void)eventType;
    (void)userData;

    int width  = e->windowInnerWidth;
    int height = e->windowInnerHeight;

    if (width <= 0 || height <= 0) {
        return EM_TRUE;
    }

    printf("[resize] windowInner = %d x %d\n", width, height);
    emscripten_set_canvas_element_size("#canvas", width, height);

    if (g_ctx) {
        emscripten_webgl_make_context_current(g_ctx);
        glViewport(0, 0, width, height);
    }

    return EM_TRUE;
}

EMSCRIPTEN_KEEPALIVE
int initWebGL(void)
{
    printf("[initWebGL] starting\n");

    /* 0) Create and activate a WebGL2 context on <canvas id="canvas"> */
    if (!g_ctx) {
        EmscriptenWebGLContextAttributes attr;
        emscripten_webgl_init_context_attributes(&attr);
        attr.majorVersion = 2;
        attr.minorVersion = 0;
        attr.depth        = EM_FALSE;

        printf("[initWebGL] creating WebGL2 context...\n");
        g_ctx = emscripten_webgl_create_context("#canvas", &attr);
        if (g_ctx <= 0) {
            printf("[initWebGL] ❌ emscripten_webgl_create_context failed (%lu)\n",
                   (unsigned long)g_ctx);
            return 0;
        }
    }

    emscripten_webgl_make_context_current(g_ctx);

    /* Make canvas pixel size match its CSS size (of the canvas element) */
    double css_w = 0.0;
    double css_h = 0.0;
    EMSCRIPTEN_RESULT css_res =
        emscripten_get_element_css_size("#canvas", &css_w, &css_h);

    int pixel_w = 0;
    int pixel_h = 0;

    if (css_res == EMSCRIPTEN_RESULT_SUCCESS &&
        css_w > 0.0 && css_h > 0.0) {
        pixel_w = (int)css_w;
        pixel_h = (int)css_h;
        printf("[initWebGL] canvas CSS size = %.1f x %.1f\n", css_w, css_h);
    } else {
        /* Fallback if CSS query fails */
        pixel_w = 800;
        pixel_h = 600;
        printf("[initWebGL] css size query failed, using fallback %d x %d\n",
               pixel_w, pixel_h);
    }

    emscripten_set_canvas_element_size("#canvas", pixel_w, pixel_h);

    /* Match viewport to canvas size */
    int w = 0;
    int h = 0;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    printf("[initWebGL] canvas pixel size = %d x %d\n", w, h);
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;
    glViewport(0, 0, w, h);

    /* Register resize callback so canvas tracks browser size */
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                   NULL,
                                   EM_FALSE,
                                   resize_callback);

    /* 1) Compile & link shaders */
    printf("[initWebGL] compiling shaders...\n");
    GLuint vs = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

#if 0
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        char *log = (char *)malloc((size_t)len);
        if (log) {
            glGetProgramInfoLog(program, len, NULL, log);
            fprintf(stderr, "Program link error:\n%s\n", log);
            free(log);
        }
        return 0;
    }
#endif

    /* 2) Build a polygon in NDC space */
    polygon_init(&g_polygon);

    printf("[initWebGL] building regular hexagon polygon...\n");
    if (!polygon_make_regular_ngon(&g_polygon, 6U, 0.5)) {
        printf("[initWebGL] ⚠ polygon_make_regular_ngon failed, falling back to triangle\n");

        /* Simple triangle fallback */
        float tri[] = {
            -0.5f, -0.5f,
             0.5f, -0.5f,
             0.0f,  0.5f
        };
        vertexCount = 3;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof tri, tri, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                              (GLsizei)(2 * (GLint)sizeof(float)), (const void *)0);
    } else {
        if (g_polygon.count == 0U || g_polygon.vertices == NULL) {
            fprintf(stderr, "[initWebGL] polygon is empty\n");
            return 0;
        }

        printf("[initWebGL] polygon has %zu vertices\n", g_polygon.count);
        vertexCount = (GLsizei)g_polygon.count;

        /* Flatten polygon vertices to float buffer [x0,y0,x1,y1,...] */
        float *verts = (float *)malloc(sizeof(float) * 2U * (size_t)vertexCount);
        if (!verts) {
            fprintf(stderr, "[initWebGL] malloc failed for verts\n");
            return 0;
        }

        for (size_t i = 0U; i < (size_t)vertexCount; ++i) {
            verts[i * 2U + 0U] = (float)g_polygon.vertices[i].x;
            verts[i * 2U + 1U] = (float)g_polygon.vertices[i].y;
        }

        /* 3) Upload into VAO/VBO */
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)(sizeof(float) * 2U * (size_t)vertexCount),
                     verts,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                              (GLsizei)(2 * (GLint)sizeof(float)), (const void *)0);

        free(verts);
    }

    /* Black background (screen stays black) */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    printf("[initWebGL] done\n");
    return 1; /* success */
}

/* per-frame draw function */
static void tick(void)
{
    if (g_ctx) {
        emscripten_webgl_make_context_current(g_ctx);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (program != 0 && vao != 0 && vertexCount > 0) {
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
    }
}

EMSCRIPTEN_KEEPALIVE
void startMainLoop(void)
{
    printf("[startMainLoop] called\n");

    if (g_main_loop_started) {
        printf("[startMainLoop] main loop already started, ignoring\n");
        return;
    }

    g_main_loop_started = true;
    printf("[startMainLoop] entering main loop\n");

    /* 0 = browser-driven fps, 1 = simulate infinite loop */
    emscripten_set_main_loop(tick, 0, 1);
}
