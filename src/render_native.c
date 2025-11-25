/* render_native.c - SDL3 + OpenGL renderer for native platforms */

#ifdef USE_SDL3

#include "polygon.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

/* ========================================================================= */
/* Global State                                                              */
/* ========================================================================= */

static SDL_Window   *g_window   = NULL;
static SDL_GLContext g_context  = 0;
static bool          g_running  = true;

static GLuint  g_program     = 0;
static GLuint  g_vao         = 0;
static GLuint  g_vbo         = 0;
static GLsizei g_vertexCount = 0;

static Polygon  g_polygon;
static Point2D *g_baseVerts        = NULL;
static float   *g_vertexData       = NULL;
static size_t   g_vertexFloatCount = 0;

/* ========================================================================= */
/* Shader Sources (OpenGL 3.3 Core)                                         */
/* ========================================================================= */

static const char *VERT_SRC =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "void main() {\n"
    "  gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const char *FRAG_SRC =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "  FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

/* ========================================================================= */
/* Shader Compilation                                                        */
/* ========================================================================= */

static GLuint compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compilation error:\n%s\n", log);
    }
    
    return shader;
}

/* ========================================================================= */
/* Initialization                                                            */
/* ========================================================================= */

int initRenderer(void) {
    printf("[initRenderer] Initializing SDL3 + OpenGL...\n");
    
    /* Initialize SDL */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }
    
    /* Request OpenGL 3.3 Core Profile */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    /* Create window */
    g_window = SDL_CreateWindow(
        "Polygon Renderer - SDL3",
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    
    /* Create OpenGL context */
    g_context = SDL_GL_CreateContext(g_window);
    if (!g_context) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 0;
    }
    
    /* Enable VSync */
    SDL_GL_SetSwapInterval(1);
    
    /* Set initial viewport */
    int w = 0, h = 0;
    SDL_GetWindowSize(g_window, &w, &h);
    glViewport(0, 0, w, h);
    
    printf("[initRenderer] OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("[initRenderer] GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    /* Compile shaders */
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    
    g_program = glCreateProgram();
    glAttachShader(g_program, vs);
    glAttachShader(g_program, fs);
    glLinkProgram(g_program);
    
    GLint linked = 0;
    glGetProgramiv(g_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(g_program, sizeof(log), NULL, log);
        fprintf(stderr, "Shader linking error:\n%s\n", log);
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    /* Create polygon */
    polygon_init(&g_polygon);
    if (!polygon_make_regular_ngon(&g_polygon, 6U, 0.3)) {
        fprintf(stderr, "[initRenderer] Failed to create polygon\n");
        return 0;
    }
    
    printf("[initRenderer] Created hexagon with %zu vertices\n", g_polygon.count);
    
    g_vertexCount = (GLsizei)g_polygon.count;
    g_vertexFloatCount = (size_t)g_vertexCount * 2U;
    
    /* Store base vertices */
    g_baseVerts = (Point2D *)malloc(sizeof(Point2D) * g_polygon.count);
    if (!g_baseVerts) {
        fprintf(stderr, "[initRenderer] malloc failed\n");
        return 0;
    }
    
    for (size_t i = 0; i < g_polygon.count; ++i) {
        g_baseVerts[i] = g_polygon.vertices[i];
    }
    
    /* Allocate vertex buffer */
    g_vertexData = (float *)malloc(sizeof(float) * g_vertexFloatCount);
    if (!g_vertexData) {
        fprintf(stderr, "[initRenderer] malloc failed\n");
        return 0;
    }
    
    for (size_t i = 0; i < (size_t)g_vertexCount; ++i) {
        g_vertexData[2U * i + 0U] = (float)g_polygon.vertices[i].x;
        g_vertexData[2U * i + 1U] = (float)g_polygon.vertices[i].y;
    }
    
    /* Create VAO/VBO */
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * g_vertexFloatCount,
                 g_vertexData,
                 GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), (void *)0);
    
    glBindVertexArray(0);
    
    /* OpenGL state */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    printf("[initRenderer] Initialization complete\n");
    return 1;
}

/* ========================================================================= */
/* Event Processing                                                          */
/* ========================================================================= */

static void handleEvents(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                g_running = false;
                break;
                
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    g_running = false;
                }
                break;
                
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                int w = 0, h = 0;
                SDL_GetWindowSize(g_window, &w, &h);
                glViewport(0, 0, w, h);
                printf("[handleEvents] Window resized to %dx%d\n", w, h);
                break;
            }
        }
    }
}

/* ========================================================================= */
/* Rendering                                                                 */
/* ========================================================================= */

static void renderFrame(void) {
    static Uint64 frameCount = 0;
    frameCount++;
    
    /* Animate polygon */
    if (polygon_is_valid(&g_polygon) && g_baseVerts && g_vertexData) {
        double t = (double)frameCount;
        double angle = 0.05 * t;
        double orbitR = 0.6;
        double tx = orbitR * cos(0.01 * t);
        double ty = orbitR * sin(0.013 * t);
        
        double c = cos(angle);
        double s = sin(angle);
        
        for (size_t i = 0; i < (size_t)g_vertexCount; ++i) {
            double x0 = g_baseVerts[i].x;
            double y0 = g_baseVerts[i].y;
            
            double xr = c * x0 - s * y0;
            double yr = s * x0 + c * y0;
            
            double x = xr + tx;
            double y = yr + ty;
            
            g_polygon.vertices[i].x = x;
            g_polygon.vertices[i].y = y;
            
            g_vertexData[2U * i + 0U] = (float)x;
            g_vertexData[2U * i + 1U] = (float)y;
        }
        
        /* Update VBO */
        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                       sizeof(float) * g_vertexFloatCount,
                       g_vertexData);
        
        /* Log periodically */
        if ((frameCount % 60) == 0) {
            Point2D *p0 = &g_polygon.vertices[0];
            double per = polygon_perimeter(&g_polygon);
            printf("[frame %llu] first=(%.3f, %.3f) perimeter=%.3f\n",
                   (unsigned long long)frameCount, p0->x, p0->y, per);
        }
    }
    
    /* Clear and draw */
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (g_program && g_vao && g_vertexCount > 0) {
        glUseProgram(g_program);
        glBindVertexArray(g_vao);
        glDrawArrays(GL_LINE_LOOP, 0, g_vertexCount);
    }
    
    SDL_GL_SwapWindow(g_window);
}

/* ========================================================================= */
/* Main Loop                                                                 */
/* ========================================================================= */

void startMainLoop(void) {
    printf("[startMainLoop] Entering main loop\n");
    
    while (g_running) {
        handleEvents();
        renderFrame();
        
        /* Simple frame rate limiting (16ms ≈ 60 FPS) */
        SDL_Delay(16);
    }
    
    printf("[startMainLoop] Exiting main loop\n");
}

/* ========================================================================= */
/* Cleanup                                                                   */
/* ========================================================================= */

void cleanupRenderer(void) {
    printf("[cleanupRenderer] Cleaning up resources\n");
    
    free(g_vertexData);
    free(g_baseVerts);
    polygon_clear(&g_polygon);
    
    if (g_vbo) glDeleteBuffers(1, &g_vbo);
    if (g_vao) glDeleteVertexArrays(1, &g_vao);
    if (g_program) glDeleteProgram(g_program);
    
    if (g_context) SDL_GL_DestroyContext(g_context);
    if (g_window) SDL_DestroyWindow(g_window);
    
    SDL_Quit();
}

#endif /* USE_SDL3 */