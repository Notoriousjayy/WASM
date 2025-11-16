#ifndef _H_MATH_MATRICES_
#define _H_MATH_MATRICES_

/*
  C23 version of matrices.h

  - Row-major layout preserved
  - Left-handed projection conventions preserved (see Projection / Ortho)
  - Matrix types and member names (_11, _12, ..., _44) unchanged
*/

#include <stdbool.h>

#ifndef NO_EXTRAS
#include <stdio.h>
#endif

#include "vectors.h"

/* ============================================================================
 *  Matrix types
 * ==========================================================================*/

typedef struct mat2 {
    union {
        struct {
            float _11, _12,
                  _21, _22;
        };
        float asArray[4];
    };
} mat2;

typedef struct mat3 {
    union {
        struct {
            float _11, _12, _13,
                  _21, _22, _23,
                  _31, _32, _33;
        };
        float asArray[9];
    };
} mat3;

typedef struct mat4 {
    union {
        struct {
            float _11, _12, _13, _14,
                  _21, _22, _23, _24,
                  _31, _32, _33, _34,
                  _41, _42, _43, _44;
        };
        float asArray[16];
    };
} mat4;

/* ============================================================================
 *  Small helpers (convenience constructors and row access helpers)
 *  (Declarations only; implementations live in matrices.c)
 * ==========================================================================*/

/* mat2 helpers */
mat2 mat2_identity(void);
mat2 mat2_make(float f11, float f12,
               float f21, float f22);
float       *mat2_row(mat2 *m, int row);
const float *mat2_row_const(const mat2 *m, int row);

/* mat3 helpers */
mat3 mat3_identity(void);
mat3 mat3_make(float f11, float f12, float f13,
               float f21, float f22, float f23,
               float f31, float f32, float f33);
float       *mat3_row(mat3 *m, int row);
const float *mat3_row_const(const mat3 *m, int row);

/* mat4 helpers */
mat4 mat4_identity(void);
mat4 mat4_make(float f11, float f12, float f13, float f14,
               float f21, float f22, float f23, float f24,
               float f31, float f32, float f33, float f34,
               float f41, float f42, float f43, float f44);
float       *mat4_row(mat4 *m, int row);
const float *mat4_row_const(const mat4 *m, int row);

/* ============================================================================
 *  Extras: comparisons and printing helpers
 * ==========================================================================*/

#ifndef NO_EXTRAS
bool mat2_equal(mat2 l, mat2 r);
bool mat3_equal(mat3 l, mat3 r);
bool mat4_equal(mat4 l, mat4 r);

bool mat2_not_equal(mat2 l, mat2 r);
bool mat3_not_equal(mat3 l, mat3 r);
bool mat4_not_equal(mat4 l, mat4 r);

/* Formatted printing helpers */
void mat2_fprintf(FILE *stream, const mat2 *m);
void mat3_fprintf(FILE *stream, const mat3 *m);
void mat4_fprintf(FILE *stream, const mat4 *m);
#endif /* NO_EXTRAS */

/* ============================================================================
 *  Transpose
 * ==========================================================================*/

/* Raw transpose of an arbitrary float matrix */
void Transpose(const float *srcMat, float *dstMat, int srcRows, int srcCols);

/* Dimension-specific versions */
mat2 mat2_transpose(mat2 matrix);
mat3 mat3_transpose(mat3 matrix);
mat4 mat4_transpose(mat4 matrix);

/* ============================================================================
 *  Scalar multiply
 * ==========================================================================*/

mat2 mat2_mul_scalar(mat2 matrix, float scalar);
mat3 mat3_mul_scalar(mat3 matrix, float scalar);
mat4 mat4_mul_scalar(mat4 matrix, float scalar);

/* ============================================================================
 *  Matrix multiply
 * ==========================================================================*/

/* Generic row-major multiply: out = matA (aRows x aCols) * matB (bRows x bCols)
   Returns false if dimensions are incompatible. */
bool Multiply(float *out,
              const float *matA, int aRows, int aCols,
              const float *matB, int bRows, int bCols);

/* Dimension-specific versions */
mat2 mat2_mul(mat2 matrixA, mat2 matrixB);
mat3 mat3_mul(mat3 matrixA, mat3 matrixB);
mat4 mat4_mul(mat4 matrixA, mat4 matrixB);

/* ============================================================================
 *  Minors, cofactors, determinant, adjugate, inverse
 * ==========================================================================*/

mat2 mat3_cut(mat3 mat, int row, int col);   /* Cut 3x3 -> 2x2  */
mat3 mat4_cut(mat4 mat, int row, int col);   /* Cut 4x4 -> 3x3  */

void Cofactor(float *out, const float *minor, int rows, int cols); /* raw */

/* mat2 */
mat2  mat2_minor(mat2 mat);
mat2  mat2_cofactor(mat2 mat);
float mat2_determinant(mat2 matrix);
mat2  mat2_adjugate(mat2 mat);
mat2  mat2_inverse(mat2 mat);

/* mat3 */
mat3  mat3_minor(mat3 mat);
mat3  mat3_cofactor(mat3 mat);
float mat3_determinant(mat3 mat);
mat3  mat3_adjugate(mat3 mat);
mat3  mat3_inverse(mat3 mat);

/* mat4 */
mat4  mat4_minor(mat4 mat);
mat4  mat4_cofactor(mat4 mat);
float mat4_determinant(mat4 mat);
mat4  mat4_adjugate(mat4 mat);
mat4  mat4_inverse(mat4 mat);

/* ============================================================================
 *  Row-major / column-major conversions
 * ==========================================================================*/

mat4 mat4_to_column_major(mat4 mat);
mat3 mat3_to_column_major(mat3 mat);

mat4 mat4_from_column_major_mat4(mat4 mat);
mat3 mat3_from_column_major_mat3(mat3 mat);
mat4 mat4_from_column_major_array(const float *mat);

/* ============================================================================
 *  Translation
 * ==========================================================================*/

mat4 mat4_translation_xyz(float x, float y, float z);
mat4 mat4_translation_vec3(vec3 pos);
vec3 mat4_get_translation(mat4 mat);

#ifndef NO_EXTRAS
/* Optional aliases */
mat4 mat4_translate_xyz(float x, float y, float z);
mat4 mat4_translate_vec3(vec3 pos);
#endif

mat4 mat4_from_mat3(mat3 mat);

/* ============================================================================
 *  Scale
 * ==========================================================================*/

mat4 mat4_scale_xyz(float x, float y, float z);
mat4 mat4_scale_vec3(vec3 vec);
vec3 mat4_get_scale(mat4 mat);

/* ============================================================================
 *  Rotation
 * ==========================================================================*/

/* X, Y, Z (same semantics as original Rotation and Rotation3x3) */
mat4 Rotation(float pitch, float yaw, float roll);
mat3 Rotation3x3(float pitch, float yaw, float roll);

#ifndef NO_EXTRAS
mat2 Rotation2x2(float angle);
mat4 YawPitchRoll(float yaw, float pitch, float roll); /* Y, X, Z */
#endif

mat4 XRotation(float angle);
mat3 XRotation3x3(float angle);

mat4 YRotation(float angle);
mat3 YRotation3x3(float angle);

mat4 ZRotation(float angle);
mat3 ZRotation3x3(float angle);

/* ============================================================================
 *  Orthogonalization (extras)
 * ==========================================================================*/

#ifndef NO_EXTRAS
mat4 mat4_orthogonalize(mat4 mat);
mat3 mat3_orthogonalize(mat3 mat);
#endif

/* ============================================================================
 *  Axis-angle rotations
 * ==========================================================================*/

mat4 AxisAngle(vec3 axis, float angle);
mat3 AxisAngle3x3(vec3 axis, float angle);

/* ============================================================================
 *  Vector/matrix multiplication
 * ==========================================================================*/

/* vec treated as point (w = 1) */
vec3 MultiplyPoint(vec3 vec, mat4 mat);

/* vec treated as direction (w = 0) – two explicit versions */
vec3 mat4_multiply_vector(vec3 vec, mat4 mat);
vec3 mat3_multiply_vector(vec3 vec, mat3 mat);

/* ============================================================================
 *  Composite transforms
 * ==========================================================================*/

/* Euler (scale, then Euler XYZ rotation, then translate) */
mat4 TransformEuler(vec3 scale, vec3 eulerRotation, vec3 translate);

/* Axis-angle (scale, then axis-angle rotation, then translate) */
mat4 TransformAxisAngle(vec3 scale, vec3 rotationAxis, float rotationAngle, vec3 translate);

/* ============================================================================
 *  View / projection
 * ==========================================================================*/

mat4 LookAt(vec3 position, vec3 target, vec3 up);
mat4 Projection(float fov, float aspect, float zNear, float zFar);
mat4 Ortho(float left, float right, float bottom, float top, float zNear, float zFar);

/* ============================================================================
 *  Decompose rotation (mat3 -> Euler or similar)
 * ==========================================================================*/

vec3 Decompose(mat3 rot);

/* ============================================================================
 *  Fast inverse (extras)
 * ==========================================================================*/

#ifndef NO_EXTRAS
mat3 mat3_fast_inverse(mat3 mat);
mat4 mat4_fast_inverse(mat4 mat);
#endif

#endif /* _H_MATH_MATRICES_ */
