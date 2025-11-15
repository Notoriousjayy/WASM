/* render.c — polygon renderer */

#include "render.h"
#include "polygon.h"

#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>

static GLuint   program     = 0;
static GLuint   vao         = 0;
static GLuint   vbo         = 0;
static GLsizei  vertexCount = 0;
static Polygon  g_polygon;

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

    /* Optional: check compile status for debugging */
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

EMSCRIPTEN_KEEPALIVE
int initWebGL(void)
{
    /* 0) Create and activate a WebGL2 context on <canvas id="canvas"> */
    static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = 0;
    if (!ctx) {
        EmscriptenWebGLContextAttributes attr;
        emscripten_webgl_init_context_attributes(&attr);
        attr.majorVersion = 2;
        attr.minorVersion = 0;
        attr.depth        = EM_FALSE;

        ctx = emscripten_webgl_create_context("#canvas", &attr);
        if (ctx <= 0) {
            printf("❌ emscripten_webgl_create_context failed (%lu)\n",
                   (unsigned long)ctx);
            return 0;
        }

        emscripten_webgl_make_context_current(ctx);

        /* Match viewport to canvas size */
        int w = 0, h = 0;
        emscripten_get_canvas_element_size("#canvas", &w, &h);
        glViewport(0, 0, w, h);
    }

    /* 1) Compile & link shaders */
    GLuint vs = compileShader(GL_VERTEX_SHADER,   VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    /* Optional: check link status */
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

    /* Example: regular hexagon with radius 0.5 */
    if (!polygon_make_regular_ngon(&g_polygon, 6, 0.5f)) {
        fprintf(stderr, "Failed to build polygon geometry\n");
        return 0;
    }

    if (g_polygon.count == 0U || g_polygon.vertices == NULL) {
        fprintf(stderr, "Polygon is empty\n");
        return 0;
    }

    vertexCount = (GLsizei)g_polygon.count;

    /* Flatten polygon vertices to float buffer [x0,y0,x1,y1,...] */
    float *verts = (float *)malloc(sizeof(float) * 2U * (size_t)vertexCount);
    if (!verts) {
        fprintf(stderr, "malloc failed for verts\n");
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
                 sizeof(float) * 2U * (size_t)vertexCount,
                 verts,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * (GLint)sizeof(float), 0);

    free(verts);

    /* White background */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    return 1; /* success */
}

/* per-frame draw function */
static void tick(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glBindVertexArray(vao);

    /* Draw outlined polygon */
    glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
}

EMSCRIPTEN_KEEPALIVE
void startMainLoop(void)
{
    /* 0 = browser-driven fps, 1 = simulate infinite loop */
    emscripten_set_main_loop(tick, 0, 1);
}
