/* render.c — drop-in replacement */

#include "render.h"
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static GLuint   program     = 0;
static GLuint   vao         = 0;
static GLuint   vbo         = 0;
static GLsizei  vertexCount = 0;

/* Simple pass-through vertex shader */
static const char* VERT_SRC =
  "#version 300 es\n"
  "layout(location = 0) in vec2 aPos;\n"
  "void main() {\n"
  "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
  "}\n";

/* Solid-red fragment shader */
static const char* FRAG_SRC =
  "#version 300 es\n"
  "precision mediump float;\n"
  "out vec4 outColor;\n"
  "void main() {\n"
  "  outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
  "}\n";

static GLuint compileShader(GLenum type, const char* src) {
  GLuint sh = glCreateShader(type);
  glShaderSource(sh, 1, &src, NULL);
  glCompileShader(sh);
  /* (Optional) Check compile status here */
  return sh;
}

EMSCRIPTEN_KEEPALIVE
int initWebGL(void) {
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
      printf("❌ emscripten_webgl_create_context failed (%lu)\n", (unsigned long)ctx);
      return 0;
    }
    emscripten_webgl_make_context_current(ctx);
    /* Match viewport to canvas size */
    int w, h;
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
  /* (Optional) Check link status here */

  /* 2) Build a unit-circle as a TRIANGLE_FAN around (0,0) */
  const int SEGMENTS = 64;
  vertexCount = SEGMENTS + 2;
  float *verts = malloc(sizeof(float) * 2 * vertexCount);

  /* center point */
  verts[0] = 0.0f;
  verts[1] = 0.0f;

  /* circumference points */
  for (int i = 0; i <= SEGMENTS; ++i) {
    float t     = (float)i / (float)SEGMENTS;
    float theta = 2.0f * M_PI * t;
    verts[(i+1)*2 + 0] = cosf(theta) * 0.5f;
    verts[(i+1)*2 + 1] = sinf(theta) * 0.5f;
  }

  /* 3) Upload into VAO/VBO */
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float)*2*vertexCount,
               verts,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

  free(verts);

  /* 4) Set clear color (white background) */
  glClearColor(1.0, 1.0, 1.0, 1.0);

  return 1;  /* success */
}

/* per-frame draw function */
static void tick(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(program);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
}

EMSCRIPTEN_KEEPALIVE
void startMainLoop(void) {
  /* 0 = browser-driven fps, 1 = simulate infinite loop */
  emscripten_set_main_loop(tick, 0, 1);
}
