/* render.c — polygon renderer with obvious motion + logging */

#include "render.h"
#include "polygon.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#else
// For native builds, define OpenGL types as stubs
typedef unsigned int GLuint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef int GLint;
typedef float GLfloat;
typedef unsigned char GLboolean;
typedef int GLsizeiptr;

#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ARRAY_BUFFER 0x8892
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FLOAT 0x1406
#define GL_FALSE 0
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_LINE_LOOP 0x0002

#warning "render.c is designed for WebGL2/Emscripten - native OpenGL not implemented"
#endif

/* ------------------------------------------------------------------------- */
/* Global WebGL / render state                                               */
/* ------------------------------------------------------------------------- */

#ifdef __EMSCRIPTEN__
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_ctx = 0;

static GLuint   program     = 0;
static GLuint   vao         = 0;
static GLuint   vbo         = 0;
static GLsizei  vertexCount = 0;

/* CPU-side mirror of vertex data (x0,y0,x1,y1,...) for VBO updates */
static float   *g_vertexData       = NULL;
static size_t   g_vertexFloatCount = 0;
#endif

static Polygon  g_polygon;             /* animated polygon */
static Point2D *g_baseVerts = NULL;    /* original (untransformed) vertices */
static bool     g_main_loop_started = false;

/* ------------------------------------------------------------------------- */
/* Shaders                                                                   */
/* ------------------------------------------------------------------------- */

#ifdef __EMSCRIPTEN__
static const char *VERT_SRC =
    "#version 300 es\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main() {\n"
    "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

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

/* ------------------------------------------------------------------------- */
/* Resize callback: keep canvas size == browser window size                  */
/* ------------------------------------------------------------------------- */

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
#endif /* __EMSCRIPTEN__ */

/* ------------------------------------------------------------------------- */
/* initWebGL                                                                 */
/* ------------------------------------------------------------------------- */

EMSCRIPTEN_KEEPALIVE
int initWebGL(void)
{
#ifdef __EMSCRIPTEN__
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

    /* 2) Build a polygon in NDC space */
    polygon_init(&g_polygon);

    printf("[initWebGL] building regular hexagon polygon...\n");
    if (!polygon_make_regular_ngon(&g_polygon, 6U, 0.3)) { /* slightly smaller radius */
        printf("[initWebGL] ⚠ polygon_make_regular_ngon failed\n");
        return 0;
    }

    if (!polygon_is_valid(&g_polygon)) {
        printf("[initWebGL] polygon not valid\n");
        return 0;
    }

    printf("[initWebGL] polygon has %zu vertices\n", g_polygon.count);
    vertexCount        = (GLsizei)g_polygon.count;
    g_vertexFloatCount = (size_t)vertexCount * 2U;

    /* Store base vertices (untransformed) for animation */
    free(g_baseVerts);
    g_baseVerts = (Point2D *)malloc(sizeof(Point2D) * g_polygon.count);
    if (!g_baseVerts) {
        fprintf(stderr, "[initWebGL] malloc failed for g_baseVerts\n");
        return 0;
    }
    for (size_t i = 0; i < g_polygon.count; ++i) {
        g_baseVerts[i] = g_polygon.vertices[i];
    }

    /* Allocate CPU-side vertex buffer */
    free(g_vertexData);
    g_vertexData = (float *)malloc(sizeof(float) * g_vertexFloatCount);
    if (!g_vertexData) {
        fprintf(stderr, "[initWebGL] malloc failed for g_vertexData\n");
        return 0;
    }

    for (size_t i = 0U; i < (size_t)vertexCount; ++i) {
        g_vertexData[2U * i + 0U] = (float)g_polygon.vertices[i].x;
        g_vertexData[2U * i + 1U] = (float)g_polygon.vertices[i].y;
    }

    /* 3) Upload into VAO/VBO */
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(sizeof(float) * g_vertexFloatCount),
                 g_vertexData,
                 GL_DYNAMIC_DRAW); /* dynamic, since we'll update each tick */

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          (GLsizei)(2 * (GLint)sizeof(float)), (const void *)0);

    /* Black background */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    printf("[initWebGL] done\n");
    return 1; /* success */
#else
    printf("[initWebGL] Native build - WebGL not available\n");
    return 0;
#endif
}

/* ------------------------------------------------------------------------- */
/* Per-frame draw function                                                   */
/* ------------------------------------------------------------------------- */

#ifdef __EMSCRIPTEN__
static void tick(void)
{
    static int frameCount = 0;

    /* Make sure our WebGL context is current before drawing */
    if (g_ctx) {
        emscripten_webgl_make_context_current(g_ctx);
    }

    frameCount++;

    if (polygon_is_valid(&g_polygon) &&
        g_baseVerts != NULL &&
        g_vertexData != NULL &&
        g_vertexFloatCount == (size_t)vertexCount * 2U)
    {
        /* Time-based angle and translation for very obvious motion */
        double t      = (double)frameCount;
        double angle  = 0.05 * t;                 /* fast rotation */
        double orbitR = 0.6;                      /* orbit radius in NDC */
        double tx     = orbitR * cos(0.01 * t);   /* x offset */
        double ty     = orbitR * sin(0.013 * t);  /* y offset */

        double c = cos(angle);
        double s = sin(angle);

        /* Build animated polygon from base vertices */
        for (size_t i = 0; i < (size_t)vertexCount; ++i) {
            double x0 = g_baseVerts[i].x;
            double y0 = g_baseVerts[i].y;

            /* rotate around origin */
            double xr = c * x0 - s * y0;
            double yr = s * x0 + c * y0;

            /* then translate in an orbit */
            double x = xr + tx;
            double y = yr + ty;

            g_polygon.vertices[i].x = x;
            g_polygon.vertices[i].y = y;

            g_vertexData[2U * i + 0U] = (float)x;
            g_vertexData[2U * i + 1U] = (float)y;
        }

        /* Upload updated vertices to GPU */
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        (GLsizeiptr)(sizeof(float) * g_vertexFloatCount),
                        g_vertexData);

        /* Log first vertex and perimeter regularly so you can see motion */
        if ((frameCount % 30) == 0) {
            const Point2D *p0 = &g_polygon.vertices[0];
            double per = polygon_perimeter(&g_polygon);
            printf("[tick] frame=%d first=(%.3f, %.3f) perimeter=%.3f\n",
                   frameCount, p0->x, p0->y, per);
        }
    } else {
        if ((frameCount % 60) == 0) {
            printf("[tick] frame=%d (animation conditions not met)\n",
                   frameCount);
        }
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (program != 0 && vao != 0 && vertexCount > 0) {
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
    }
}
#endif /* __EMSCRIPTEN__ */

/* ------------------------------------------------------------------------- */
/* startMainLoop                                                             */
/* ------------------------------------------------------------------------- */

EMSCRIPTEN_KEEPALIVE
void startMainLoop(void)
{
#ifdef __EMSCRIPTEN__
    printf("[startMainLoop] called\n");

    if (g_main_loop_started) {
        printf("[startMainLoop] main loop already started, ignoring\n");
        return;
    }

    g_main_loop_started = true;
    printf("[startMainLoop] entering main loop\n");

    /* 0 = browser-driven fps, 1 = simulate infinite loop */
    emscripten_set_main_loop(tick, 0, 1);
#else
    printf("[startMainLoop] Native build - main loop not available\n");
#endif
}