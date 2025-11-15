#ifndef CYCLONE_PRECISION_H
#define CYCLONE_PRECISION_H

/*
 * Precision configuration for Cyclone – C23 version.
 *
 * This header defines:
 *   - real            : floating-point type (float or double)
 *   - REAL_MAX        : max representable real
 *   - real_sqrt       : sqrt at the correct precision
 *   - real_abs        : fabs at the correct precision
 *   - real_sin / cos  : sin / cos at the correct precision
 *   - real_exp / pow  : exp / pow at the correct precision
 *   - real_fmod       : fmod at the correct precision
 *   - real_epsilon    : machine epsilon for real
 *   - R_PI            : π at the chosen precision
 *
 * Compile-time switch:
 *   - Define CYCLONE_USE_SINGLE_PRECISION as 1 for float,
 *     otherwise double is used by default.
 */

#include <float.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Set to 1 to use float; defaults to double if not defined. */
#ifndef CYCLONE_USE_SINGLE_PRECISION
#define CYCLONE_USE_SINGLE_PRECISION 0
#endif

#if CYCLONE_USE_SINGLE_PRECISION

    /* Single precision mode */
    #ifndef SINGLE_PRECISION
    #define SINGLE_PRECISION
    #endif

    typedef float real;

    #define REAL_MAX      FLT_MAX
    #define real_sqrt     sqrtf
    #define real_abs      fabsf
    #define real_sin      sinf
    #define real_cos      cosf
    #define real_exp      expf
    #define real_pow      powf
    #define real_fmod     fmodf
    #define real_epsilon  FLT_EPSILON
    #define R_PI          3.14159f

#else

    /* Double precision mode */
    #ifndef DOUBLE_PRECISION
    #define DOUBLE_PRECISION
    #endif

    typedef double real;

    #define REAL_MAX      DBL_MAX
    #define real_sqrt     sqrt
    #define real_abs      fabs
    #define real_sin      sin
    #define real_cos      cos
    #define real_exp      exp
    #define real_pow      pow
    #define real_fmod     fmod
    #define real_epsilon  DBL_EPSILON
    #define R_PI          3.14159265358979

#endif

/*
 * Optional: exported constant for π at the chosen precision.
 * This is useful if you want a link-time symbol instead of a macro.
 */
extern const real cyclone_pi;

/*
 * Small helper functions using the configured precision.
 * These are defined in precision.c.
 */

/* Convert degrees to radians. */
real real_deg_to_rad(real degrees);

/* Convert radians to degrees. */
real real_rad_to_deg(real radians);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CYCLONE_PRECISION_H */
