/* vectors.c – C23 implementation */

#include "vectors.h"

#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#ifndef NO_EXTRAS
#include <stdio.h>
#endif

/* Optional sanity tests hook:
   If DO_SANITY_TESTS is defined and Compare.h defines CMP(x,y),
   that macro will be used. Otherwise we fall back to a default
   epsilon-based float comparison. */
#ifdef DO_SANITY_TESTS
#include "Compare.h"
#endif

#ifndef CMP
static inline bool cmp_float(float x, float y) {
    float diff    = fabsf(x - y);
    float largest = fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y)));
    return diff <= FLT_EPSILON * largest;
}
#define CMP(x, y) cmp_float((x), (y))
#endif

/* ------------------------------------------------------------------------- */
/* Angle helpers                                                             */
/* ------------------------------------------------------------------------- */

float CorrectDegrees(float degrees) {
    while (degrees > 360.0f) {
        degrees -= 360.0f;
    }
    while (degrees < -360.0f) {
        degrees += 360.0f;
    }
    return degrees;
}

float RAD2DEG(float radians) {
    float degrees = radians * 57.295754f;
    degrees       = CorrectDegrees(degrees);
    return degrees;
}

float DEG2RAD(float degrees) {
    degrees       = CorrectDegrees(degrees);
    float radians = degrees * 0.0174533f;
    return radians;
}

/* ------------------------------------------------------------------------- */
/* Constructors and indexed access                                           */
/* ------------------------------------------------------------------------- */

vec2 vec2_make(float x, float y) {
    vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

vec3 vec3_make(float x, float y, float z) {
    vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

float vec2_get(const vec2 *v, size_t index) {
    if (v == NULL || index >= 2U) {
        return 0.0f;
    }
    return v->asArray[index];
}

void vec2_set(vec2 *v, size_t index, float value) {
    if (v == NULL || index >= 2U) {
        return;
    }
    v->asArray[index] = value;
}

float vec3_get(const vec3 *v, size_t index) {
    if (v == NULL || index >= 3U) {
        return 0.0f;
    }
    return v->asArray[index];
}

void vec3_set(vec3 *v, size_t index, float value) {
    if (v == NULL || index >= 3U) {
        return;
    }
    v->asArray[index] = value;
}

/* ------------------------------------------------------------------------- */
/* Equality / inequality                                                     */
/* ------------------------------------------------------------------------- */

bool vec2_equal(vec2 l, vec2 r) {
    return CMP(l.x, r.x) && CMP(l.y, r.y);
}

bool vec3_equal(vec3 l, vec3 r) {
    return CMP(l.x, r.x) && CMP(l.y, r.y) && CMP(l.z, r.z);
}

bool vec2_not_equal(vec2 l, vec2 r) {
    return !vec2_equal(l, r);
}

bool vec3_not_equal(vec3 l, vec3 r) {
    return !vec3_equal(l, r);
}

/* ------------------------------------------------------------------------- */
/* Basic arithmetic (return-by-value)                                        */
/* ------------------------------------------------------------------------- */

vec2 vec2_add(vec2 l, vec2 r) {
    vec2 out;
    out.x = l.x + r.x;
    out.y = l.y + r.y;
    return out;
}

vec3 vec3_add(vec3 l, vec3 r) {
    vec3 out;
    out.x = l.x + r.x;
    out.y = l.y + r.y;
    out.z = l.z + r.z;
    return out;
}

vec2 vec2_sub(vec2 l, vec2 r) {
    vec2 out;
    out.x = l.x - r.x;
    out.y = l.y - r.y;
    return out;
}

vec3 vec3_sub(vec3 l, vec3 r) {
    vec3 out;
    out.x = l.x - r.x;
    out.y = l.y - r.y;
    out.z = l.z - r.z;
    return out;
}

/* Component-wise multiply */

vec2 vec2_mul(vec2 l, vec2 r) {
    vec2 out;
    out.x = l.x * r.x;
    out.y = l.y * r.y;
    return out;
}

vec3 vec3_mul(vec3 l, vec3 r) {
    vec3 out;
    out.x = l.x * r.x;
    out.y = l.y * r.y;
    out.z = l.z * r.z;
    return out;
}

/* Scalar multiply */

vec2 vec2_mul_scalar(vec2 v, float s) {
    vec2 out;
    out.x = v.x * s;
    out.y = v.y * s;
    return out;
}

vec3 vec3_mul_scalar(vec3 v, float s) {
    vec3 out;
    out.x = v.x * s;
    out.y = v.y * s;
    out.z = v.z * s;
    return out;
}

#ifndef NO_EXTRAS
/* Component-wise divide */

vec2 vec2_div(vec2 l, vec2 r) {
    vec2 out;
    out.x = l.x / r.x;
    out.y = l.y / r.y;
    return out;
}

vec3 vec3_div(vec3 l, vec3 r) {
    vec3 out;
    out.x = l.x / r.x;
    out.y = l.y / r.y;
    out.z = l.z / r.z;
    return out;
}

/* Scalar divide */

vec2 vec2_div_scalar(vec2 v, float s) {
    vec2 out;
    out.x = v.x / s;
    out.y = v.y / s;
    return out;
}

vec3 vec3_div_scalar(vec3 v, float s) {
    vec3 out;
    out.x = v.x / s;
    out.y = v.y / s;
    out.z = v.z / s;
    return out;
}

/* Printing */

void vec2_print(FILE *stream, vec2 v) {
    if (stream == NULL) {
        stream = stdout;
    }
    (void)fprintf(stream, "(%f, %f)", (double)v.x, (double)v.y);
}

void vec3_print(FILE *stream, vec3 v) {
    if (stream == NULL) {
        stream = stdout;
    }
    (void)fprintf(stream, "(%f, %f, %f)", (double)v.x, (double)v.y, (double)v.z);
}
#endif /* NO_EXTRAS */

/* ------------------------------------------------------------------------- */
/* In-place compound operations                                              */
/* ------------------------------------------------------------------------- */

#ifndef NO_EXTRAS
void vec2_add_inplace(vec2 *l, vec2 r) {
    if (l == NULL) {
        return;
    }
    l->x += r.x;
    l->y += r.y;
}

void vec2_sub_inplace(vec2 *l, vec2 r) {
    if (l == NULL) {
        return;
    }
    /* NOTE: fixed a likely typo from the original C++:
       original had l.x -= r.y, which is almost certainly wrong. */
    l->x -= r.x;
    l->y -= r.y;
}

void vec2_mul_inplace(vec2 *l, vec2 r) {
    if (l == NULL) {
        return;
    }
    l->x *= r.x;
    l->y *= r.y;
}

void vec2_mul_scalar_inplace(vec2 *l, float s) {
    if (l == NULL) {
        return;
    }
    l->x *= s;
    l->y *= s;
}

void vec2_div_inplace(vec2 *l, vec2 r) {
    if (l == NULL) {
        return;
    }
    l->x /= r.x;
    l->y /= r.y;
}

void vec2_div_scalar_inplace(vec2 *l, float s) {
    if (l == NULL) {
        return;
    }
    l->x /= s;
    l->y /= s;
}

void vec3_add_inplace(vec3 *l, vec3 r) {
    if (l == NULL) {
        return;
    }
    l->x += r.x;
    l->y += r.y;
    l->z += r.z;
}

void vec3_sub_inplace(vec3 *l, vec3 r) {
    if (l == NULL) {
        return;
    }
    l->x -= r.x;
    l->y -= r.y;
    l->z -= r.z;
}

void vec3_mul_inplace(vec3 *l, vec3 r) {
    if (l == NULL) {
        return;
    }
    l->x *= r.x;
    l->y *= r.y;
    l->z *= r.z;
}

void vec3_mul_scalar_inplace(vec3 *l, float s) {
    if (l == NULL) {
        return;
    }
    l->x *= s;
    l->y *= s;
    l->z *= s;
}

void vec3_div_inplace(vec3 *l, vec3 r) {
    if (l == NULL) {
        return;
    }
    l->x /= r.x;
    l->y /= r.y;
    l->z /= r.z;
}

void vec3_div_scalar_inplace(vec3 *l, float s) {
    if (l == NULL) {
        return;
    }
    l->x /= s;
    l->y /= s;
    l->z /= s;
}
#endif /* NO_EXTRAS */

/* ------------------------------------------------------------------------- */
/* Dot products, magnitudes, distances                                      */
/* ------------------------------------------------------------------------- */

float vec2_dot(vec2 l, vec2 r) {
    return l.x * r.x + l.y * r.y;
}

float vec3_dot(vec3 l, vec3 r) {
    return l.x * r.x + l.y * r.y + l.z * r.z;
}

float vec2_magnitude_sq(vec2 v) {
    return vec2_dot(v, v);
}

float vec3_magnitude_sq(vec3 v) {
    return vec3_dot(v, v);
}

float vec2_magnitude(vec2 v) {
    return sqrtf(vec2_magnitude_sq(v));
}

float vec3_magnitude(vec3 v) {
    return sqrtf(vec3_magnitude_sq(v));
}

#ifndef NO_EXTRAS
float vec2_distance(vec2 p1, vec2 p2) {
    return vec2_magnitude(vec2_sub(p1, p2));
}

float vec3_distance(vec3 p1, vec3 p2) {
    return vec3_magnitude(vec3_sub(p1, p2));
}

float vec2_distance_sq(vec2 p1, vec2 p2) {
    return vec2_magnitude_sq(vec2_sub(p1, p2));
}

float vec3_distance_sq(vec3 p1, vec3 p2) {
    return vec3_magnitude_sq(vec3_sub(p1, p2));
}
#endif /* NO_EXTRAS */

/* ------------------------------------------------------------------------- */
/* Rotation (2D)                                                             */
/* ------------------------------------------------------------------------- */

#ifndef NO_EXTRAS
vec2 vec2_rotate(vec2 vector, float degrees) {
    float radians = DEG2RAD(degrees);
    float s       = sinf(radians);
    float c       = cosf(radians);

    vec2 out;
    out.x = vector.x * c - vector.y * s;
    out.y = vector.x * s + vector.y * c;
    return out;
}
#endif /* NO_EXTRAS */

/* ------------------------------------------------------------------------- */
/* Normalization                                                             */
/* ------------------------------------------------------------------------- */

void vec2_normalize(vec2 *v) {
    if (v == NULL) {
        return;
    }
    float mag = vec2_magnitude(*v);
    /* Original C++ code assumed non-zero vector; we keep behavior:
       division by zero will propagate NaNs/Infs. */
    float inv = 1.0f / mag;
    *v        = vec2_mul_scalar(*v, inv);
}

void vec3_normalize(vec3 *v) {
    if (v == NULL) {
        return;
    }
    float mag = vec3_magnitude(*v);
    float inv = 1.0f / mag;
    *v        = vec3_mul_scalar(*v, inv);
}

vec2 vec2_normalized(vec2 v) {
    float mag = vec2_magnitude(v);
    float inv = 1.0f / mag;
    return vec2_mul_scalar(v, inv);
}

vec3 vec3_normalized(vec3 v) {
    float mag = vec3_magnitude(v);
    float inv = 1.0f / mag;
    return vec3_mul_scalar(v, inv);
}

/* ------------------------------------------------------------------------- */
/* Cross product (3D)                                                        */
/* ------------------------------------------------------------------------- */

vec3 vec3_cross(vec3 l, vec3 r) {
    vec3 result;
    result.x = l.y * r.z - l.z * r.y;
    result.y = l.z * r.x - l.x * r.z;
    result.z = l.x * r.y - l.y * r.x;
    return result;
}

/* ------------------------------------------------------------------------- */
/* Angles between vectors (radians)                                         */
/* ------------------------------------------------------------------------- */

float vec2_angle(vec2 l, vec2 r) {
    /* Same structure as original:
       acosf(Dot(l,r) / sqrtf(MagnitudeSq(l) * MagnitudeSq(r))) */
    float num   = vec2_dot(l, r);
    float denom = sqrtf(vec2_magnitude_sq(l) * vec2_magnitude_sq(r));
    return acosf(num / denom);
}

float vec3_angle(vec3 l, vec3 r) {
    float num   = vec3_dot(l, r);
    float denom = sqrtf(vec3_magnitude_sq(l) * vec3_magnitude_sq(r));
    return acosf(num / denom);
}

/* ------------------------------------------------------------------------- */
/* Projection / perpendicular / reflection                                  */
/* ------------------------------------------------------------------------- */

vec2 vec2_project(vec2 length, vec2 direction) {
    float dot   = vec2_dot(length, direction);
    float magSq = vec2_magnitude_sq(direction);
    float scale = dot / magSq;
    return vec2_mul_scalar(direction, scale);
}

vec3 vec3_project(vec3 length, vec3 direction) {
    float dot   = vec3_dot(length, direction);
    float magSq = vec3_magnitude_sq(direction);
    float scale = dot / magSq;
    return vec3_mul_scalar(direction, scale);
}

vec2 vec2_perpendicular(vec2 length, vec2 direction) {
    return vec2_sub(length, vec2_project(length, direction));
}

vec3 vec3_perpendicular(vec3 length, vec3 direction) {
    return vec3_sub(length, vec3_project(length, direction));
}

vec2 vec2_reflect(vec2 sourceVector, vec2 normal) {
    float factor = vec2_dot(sourceVector, normal) * 2.0f;
    return vec2_sub(sourceVector, vec2_mul_scalar(normal, factor));
}

vec3 vec3_reflect(vec3 sourceVector, vec3 normal) {
    float factor = vec3_dot(sourceVector, normal) * 2.0f;
    return vec3_sub(sourceVector, vec3_mul_scalar(normal, factor));
}

/* ------------------------------------------------------------------------- */
/* Compatibility wrappers for matrices.c                                    */
/* ------------------------------------------------------------------------- */

float Dot(vec3 a, vec3 b) {
    return vec3_dot(a, b);
}

vec3 Cross(vec3 a, vec3 b) {
    return vec3_cross(a, b);
}

float Magnitude(vec3 v) {
    return vec3_magnitude(v);
}

float MagnitudeSq(vec3 v) {
    return vec3_magnitude_sq(v);
}

vec3 Normalized(vec3 v) {
    return vec3_normalized(v);
}