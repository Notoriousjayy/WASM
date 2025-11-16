#ifndef MATH_VECTORS_H_
#define MATH_VECTORS_H_

#include <stdbool.h>
#include <stddef.h>

#ifndef NO_EXTRAS
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Angle conversion helpers (functions, not macros) */
float RAD2DEG(float radians);
float DEG2RAD(float degrees);
float CorrectDegrees(float degrees);

/* Basic vector types */
typedef struct vec2 {
    union {
        struct {
            float x;
            float y;
        };
        float asArray[2];
    };
} vec2;

typedef struct vec3 {
    union {
        struct {
            float x;
            float y;
            float z;
        };
        float asArray[3];
    };
} vec3;

/* Constructors / helpers */
vec2 vec2_make(float x, float y);
vec3 vec3_make(float x, float y, float z);

/* Safe indexed access */
float vec2_get(const vec2 *v, size_t index);
void  vec2_set(vec2 *v, size_t index, float value);

float vec3_get(const vec3 *v, size_t index);
void  vec3_set(vec3 *v, size_t index, float value);

/* Arithmetic (returns new vectors) */
vec2 vec2_add(vec2 l, vec2 r);
vec3 vec3_add(vec3 l, vec3 r);

vec2 vec2_sub(vec2 l, vec2 r);
vec3 vec3_sub(vec3 l, vec3 r);

/* Component-wise multiply */
vec2 vec2_mul(vec2 l, vec2 r);
vec3 vec3_mul(vec3 l, vec3 r);

/* Scalar multiply */
vec2 vec2_mul_scalar(vec2 v, float s);
vec3 vec3_mul_scalar(vec3 v, float s);

#ifndef NO_EXTRAS
/* Component-wise divide */
vec2 vec2_div(vec2 l, vec2 r);
vec3 vec3_div(vec3 l, vec3 r);

/* Scalar divide */
vec2 vec2_div_scalar(vec2 v, float s);
vec3 vec3_div_scalar(vec3 v, float s);

/* Simple printing helpers */
void vec2_print(FILE *stream, vec2 v);
void vec3_print(FILE *stream, vec3 v);
#endif /* NO_EXTRAS */

/* Comparisons */
bool vec2_equal(vec2 l, vec2 r);
bool vec3_equal(vec3 l, vec3 r);
bool vec2_not_equal(vec2 l, vec2 r);
bool vec3_not_equal(vec3 l, vec3 r);

#ifndef NO_EXTRAS
/* In-place compound operations */
void vec2_add_inplace(vec2 *l, vec2 r);
void vec2_sub_inplace(vec2 *l, vec2 r);
void vec2_mul_inplace(vec2 *l, vec2 r);
void vec2_mul_scalar_inplace(vec2 *l, float s);
void vec2_div_inplace(vec2 *l, vec2 r);
void vec2_div_scalar_inplace(vec2 *l, float s);

void vec3_add_inplace(vec3 *l, vec3 r);
void vec3_sub_inplace(vec3 *l, vec3 r);
void vec3_mul_inplace(vec3 *l, vec3 r);
void vec3_mul_scalar_inplace(vec3 *l, float s);
void vec3_div_inplace(vec3 *l, vec3 r);
void vec3_div_scalar_inplace(vec3 *l, float s);
#endif /* NO_EXTRAS */

/* Dot product */
float vec2_dot(vec2 l, vec2 r);
float vec3_dot(vec3 l, vec3 r);

/* Length and squared length */
float vec2_magnitude(vec2 v);
float vec3_magnitude(vec3 v);

float vec2_magnitude_sq(vec2 v);
float vec3_magnitude_sq(vec3 v);

#ifndef NO_EXTRAS
/* Distance between two points */
float vec2_distance(vec2 p1, vec2 p2);
float vec3_distance(vec3 p1, vec3 p2);

/* Squared distance */
float vec2_distance_sq(vec2 p1, vec2 p2);
float vec3_distance_sq(vec3 p1, vec3 p2);

/* Rotate 2D vector by given degrees (counter-clockwise) */
vec2 vec2_rotate(vec2 v, float degrees);
#endif /* NO_EXTRAS */

/* Normalization (in-place) */
void vec2_normalize(vec2 *v);
void vec3_normalize(vec3 *v);

/* Normalized copies (does not modify input) */
vec2 vec2_normalized(vec2 v);
vec3 vec3_normalized(vec3 v);

/* 3D cross product */
vec3 vec3_cross(vec3 l, vec3 r);

/* Angle between vectors, in radians */
float vec2_angle(vec2 l, vec2 r);
float vec3_angle(vec3 l, vec3 r);

/* Projection of "length" onto "direction" */
vec2 vec2_project(vec2 length, vec2 direction);
vec3 vec3_project(vec3 length, vec3 direction);

/* Perpendicular component: length - project(length, direction) */
vec2 vec2_perpendicular(vec2 length, vec2 direction);
vec3 vec3_perpendicular(vec3 length, vec3 direction);

/* Reflection of sourceVector about normal */
vec2 vec2_reflect(vec2 sourceVector, vec2 normal);
vec3 vec3_reflect(vec3 sourceVector, vec3 normal);

#ifdef __cplusplus
}
#endif

#endif /* MATH_VECTORS_H_ */
