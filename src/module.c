// src/module.c
#include <stdio.h>
#include <emscripten/emscripten.h>

#ifdef __cplusplus
# define EXTERN extern "C"
#else
# define EXTERN
#endif

EXTERN EMSCRIPTEN_KEEPALIVE void myFunction(void) {
    printf("MyFunction Called\n");
}
