#ifndef RENDER_H
#define RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize a WebGL2 context on the <canvas id="canvas"> element.
// Returns 1 on success, 0 on failure.
int initWebGL(void);

// Starts the render loop (clears the screen each frame).
void startMainLoop(void);

#ifdef __cplusplus
}
#endif

#endif // RENDER_H
