/* main.c - Cross-platform entry point */

#include <stdio.h>

#ifdef __EMSCRIPTEN__
/* ========================================================================= */
/* Web Platform (WASM + WebGL2)                                             */
/* ========================================================================= */
int main(void) {
    printf("WebAssembly build - waiting for JavaScript initialization\n");
    return 0;
}

#else
/* ========================================================================= */
/* Native Platforms (Desktop + Mobile with SDL3 + OpenGL/ES)               */
/* ========================================================================= */

#ifdef USE_SDL3
/* Forward declarations from render_native.c */
int initRenderer(void);
void startMainLoop(void);
void cleanupRenderer(void);

#ifdef PLATFORM_ANDROID
/* Android: SDL_main is required */
#include <SDL3/SDL_main.h>
#endif

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
#ifdef PLATFORM_ANDROID
    printf("=== Android SDL3 + OpenGL ES Renderer ===\n");
#elif defined(PLATFORM_IOS)
    printf("=== iOS SDL3 + OpenGL ES Renderer ===\n");
#else
    printf("=== Desktop SDL3 + OpenGL Renderer ===\n");
#endif
    
    if (!initRenderer()) {
        fprintf(stderr, "Failed to initialize renderer\n");
        return 1;
    }
    
#ifndef PLATFORM_ANDROID
    printf("Press ESC or close window to exit\n\n");
#endif
    
    startMainLoop();
    
    cleanupRenderer();
    
    printf("\n=== Shutdown complete ===\n");
    return 0;
}

#else
/* Native build without renderer */
int main(void) {
    printf("Native build - no renderer (SDL3 disabled)\n");
    printf("To enable SDL3 rendering, install SDL3 and reconfigure:\n");
    printf("  cmake --preset native-debug -DUSE_SDL3=ON\n");
    return 0;
}
#endif /* USE_SDL3 */

#endif /* __EMSCRIPTEN__ */