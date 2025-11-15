#ifndef CYCLONE_CORE_H
#define CYCLONE_CORE_H

#include <math.h>
#include <stdbool.h>
#include "precision.h"

/*
 * C23-compatible Cyclone core interface.
 * Types + inline helpers live here; heavy lifting lives in core.c.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Global configuration
 * ============================================================
 */

/*
 * Energy threshold under which a body can go to sleep.
 */
extern real cyclone_sleep_epsilon;

/*
 * Set / get sleep epsilon.
 */
void cyclone_set_sleep_epsilon(real value);
real cyclone_get_sleep_epsilon(void);

/* ============================================================
 * Vector3
 * ============================================================
 */

typedef struct cyclone_Vector3 {
    real x;
    real y;
    real z;
    real pad; /* for 4-word alignment */
} cyclone_Vector3;

/* Alias to make porting easier if you want the old name */
typedef cyclone_Vector3 Vector3;

/* Common constants (defined in core.c) */
extern const cyclone_Vector3 CYCLONE_VECTOR3_GRAVITY;
extern const cyclone_Vector3 CYCLONE_VECTOR3_HIGH_GRAVITY;
extern const cyclone_Vector3 CYCLONE_VECTOR3_UP;
extern const cyclone_Vector3 CYCLONE_VECTOR3_RIGHT;
extern const cyclone_Vector3 CYCLONE_VECTOR3_OUT_OF_SCREEN;
extern const cyclone_Vector3 CYCLONE_VECTOR3_X;
extern const cyclone_Vector3 CYCLONE_VECTOR3_Y;
extern const cyclone_Vector3 CYCLONE_VECTOR3_Z;

/* Constructors */

static inline cyclone_Vector3
cyclone_vector3_make(real x, real y, real z)
{
    return (cyclone_Vector3){ .x = x, .y = y, .z = z, .pad = 0 };
}

static inline cyclone_Vector3
cyclone_vector3_zero(void)
{
    return cyclone_vector3_make(0, 0, 0);
}

/* Index helpers */

static inline real
cyclone_vector3_get(const cyclone_Vector3 *v, unsigned i)
{
    if (i == 0) return v->x;
    if (i == 1) return v->y;
    return v->z;
}

static inline void
cyclone_vector3_set(cyclone_Vector3 *v, unsigned i, real value)
{
    if (i == 0)      v->x = value;
    else if (i == 1) v->y = value;
    else             v->z = value;
}

/* In-place arithmetic */

static inline void
cyclone_vector3_add_inplace(cyclone_Vector3 *a, const cyclone_Vector3 *b)
{
    a->x += b->x;
    a->y += b->y;
    a->z += b->z;
}

static inline void
cyclone_vector3_sub_inplace(cyclone_Vector3 *a, const cyclone_Vector3 *b)
{
    a->x -= b->x;
    a->y -= b->y;
    a->z -= b->z;
}

static inline void
cyclone_vector3_scale_inplace(cyclone_Vector3 *v, real s)
{
    v->x *= s;
    v->y *= s;
    v->z *= s;
}

/* Pure arithmetic */

static inline cyclone_Vector3
cyclone_vector3_add(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return cyclone_vector3_make(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline cyclone_Vector3
cyclone_vector3_sub(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return cyclone_vector3_make(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline cyclone_Vector3
cyclone_vector3_scaled(cyclone_Vector3 v, real s)
{
    return cyclone_vector3_make(v.x * s, v.y * s, v.z * s);
}

/* Component-wise product */

static inline cyclone_Vector3
cyclone_vector3_component_product(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return cyclone_vector3_make(a.x * b.x, a.y * b.y, a.z * b.z);
}

static inline void
cyclone_vector3_component_product_update(cyclone_Vector3 *a, const cyclone_Vector3 *b)
{
    a->x *= b->x;
    a->y *= b->y;
    a->z *= b->z;
}

/* Cross product */

static inline cyclone_Vector3
cyclone_vector3_cross(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return cyclone_vector3_make(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline void
cyclone_vector3_cross_inplace(cyclone_Vector3 *a, const cyclone_Vector3 *b)
{
    *a = cyclone_vector3_cross(*a, *b);
}

/* Dot product */

static inline real
cyclone_vector3_dot(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* Add scaled vector */

static inline void
cyclone_vector3_add_scaled(cyclone_Vector3 *v,
                           const cyclone_Vector3 *other,
                           real scale)
{
    v->x += other->x * scale;
    v->y += other->y * scale;
    v->z += other->z * scale;
}

/* Magnitude helpers */

static inline real
cyclone_vector3_square_magnitude(const cyclone_Vector3 *v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

static inline real
cyclone_vector3_magnitude(const cyclone_Vector3 *v)
{
    return real_sqrt(cyclone_vector3_square_magnitude(v));
}

/* Trim length */

static inline void
cyclone_vector3_trim(cyclone_Vector3 *v, real size)
{
    real sq = cyclone_vector3_square_magnitude(v);
    if (sq > size * size)
    {
        real l = real_sqrt(sq);
        if (l > (real)0)
        {
            real scale = size / l;
            cyclone_vector3_scale_inplace(v, scale);
        }
    }
}

/* Normalisation */

static inline void
cyclone_vector3_normalise(cyclone_Vector3 *v)
{
    real l = cyclone_vector3_magnitude(v);
    if (l > (real)0)
    {
        real inv = ((real)1) / l;
        cyclone_vector3_scale_inplace(v, inv);
    }
}

static inline cyclone_Vector3
cyclone_vector3_unit(cyclone_Vector3 v)
{
    cyclone_vector3_normalise(&v);
    return v;
}

/* Comparisons */

static inline bool
cyclone_vector3_equal(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}

static inline bool
cyclone_vector3_not_equal(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return !cyclone_vector3_equal(a, b);
}

static inline bool
cyclone_vector3_less(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return (a.x < b.x) && (a.y < b.y) && (a.z < b.z);
}

static inline bool
cyclone_vector3_greater(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return (a.x > b.x) && (a.y > b.y) && (a.z > b.z);
}

static inline bool
cyclone_vector3_less_equal(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return (a.x <= b.x) && (a.y <= b.y) && (a.z <= b.z);
}

static inline bool
cyclone_vector3_greater_equal(cyclone_Vector3 a, cyclone_Vector3 b)
{
    return (a.x >= b.x) && (a.y >= b.y) && (a.z >= b.z);
}

/* Clear & invert */

static inline void
cyclone_vector3_clear(cyclone_Vector3 *v)
{
    v->x = (real)0;
    v->y = (real)0;
    v->z = (real)0;
}

static inline void
cyclone_vector3_invert(cyclone_Vector3 *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
}

/* ============================================================
 * Quaternion
 * ============================================================
 */

typedef struct cyclone_Quaternion {
    real r; /* real component */
    real i; /* i component */
    real j; /* j component */
    real k; /* k component */
} cyclone_Quaternion;

typedef cyclone_Quaternion Quaternion;

static inline cyclone_Quaternion
cyclone_quaternion_make(real r, real i, real j, real k)
{
    return (cyclone_Quaternion){ r, i, j, k };
}

static inline cyclone_Quaternion
cyclone_quaternion_identity(void)
{
    return cyclone_quaternion_make((real)1, (real)0, (real)0, (real)0);
}

static inline void
cyclone_quaternion_normalise(cyclone_Quaternion *q)
{
    real d = q->r * q->r + q->i * q->i + q->j * q->j + q->k * q->k;

    if (d < real_epsilon)
    {
        q->r = (real)1;
        q->i = (real)0;
        q->j = (real)0;
        q->k = (real)0;
        return;
    }

    real inv = ((real)1.0) / real_sqrt(d);
    q->r *= inv;
    q->i *= inv;
    q->j *= inv;
    q->k *= inv;
}

static inline void
cyclone_quaternion_multiply_inplace(cyclone_Quaternion *q,
                                    const cyclone_Quaternion *multiplier)
{
    cyclone_Quaternion a = *q;

    q->r = a.r * multiplier->r - a.i * multiplier->i -
           a.j * multiplier->j - a.k * multiplier->k;

    q->i = a.r * multiplier->i + a.i * multiplier->r +
           a.j * multiplier->k - a.k * multiplier->j;

    q->j = a.r * multiplier->j + a.j * multiplier->r +
           a.k * multiplier->i - a.i * multiplier->k;

    q->k = a.r * multiplier->k + a.k * multiplier->r +
           a.i * multiplier->j - a.j * multiplier->i;
}

static inline void
cyclone_quaternion_add_scaled_vector(cyclone_Quaternion *q,
                                     const cyclone_Vector3 *v,
                                     real scale)
{
    cyclone_Quaternion p = {
        .r = (real)0,
        .i = v->x * scale,
        .j = v->y * scale,
        .k = v->z * scale
    };

    cyclone_quaternion_multiply_inplace(&p, q);

    q->r += p.r * ((real)0.5);
    q->i += p.i * ((real)0.5);
    q->j += p.j * ((real)0.5);
    q->k += p.k * ((real)0.5);
}

static inline void
cyclone_quaternion_rotate_by_vector(cyclone_Quaternion *q,
                                    const cyclone_Vector3 *v)
{
    cyclone_Quaternion p = { (real)0, v->x, v->y, v->z };
    cyclone_quaternion_multiply_inplace(q, &p);
}

/* ============================================================
 * Matrix4 – 3x4 transform matrix
 * ============================================================
 */

typedef struct cyclone_Matrix4 {
    real data[12];
} cyclone_Matrix4;

typedef cyclone_Matrix4 Matrix4;

static inline cyclone_Matrix4
cyclone_matrix4_identity(void)
{
    return (cyclone_Matrix4){
        .data = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0
        }
    };
}

static inline void
cyclone_matrix4_set_diagonal(cyclone_Matrix4 *m, real a, real b, real c)
{
    m->data[0]  = a;
    m->data[5]  = b;
    m->data[10] = c;
}

static inline cyclone_Matrix4
cyclone_matrix4_mul(const cyclone_Matrix4 *m, const cyclone_Matrix4 *o)
{
    cyclone_Matrix4 result;

    result.data[0]  = (o->data[0]*m->data[0]) + (o->data[4]*m->data[1]) + (o->data[8]*m->data[2]);
    result.data[4]  = (o->data[0]*m->data[4]) + (o->data[4]*m->data[5]) + (o->data[8]*m->data[6]);
    result.data[8]  = (o->data[0]*m->data[8]) + (o->data[4]*m->data[9]) + (o->data[8]*m->data[10]);

    result.data[1]  = (o->data[1]*m->data[0]) + (o->data[5]*m->data[1]) + (o->data[9]*m->data[2]);
    result.data[5]  = (o->data[1]*m->data[4]) + (o->data[5]*m->data[5]) + (o->data[9]*m->data[6]);
    result.data[9]  = (o->data[1]*m->data[8]) + (o->data[5]*m->data[9]) + (o->data[9]*m->data[10]);

    result.data[2]  = (o->data[2]*m->data[0]) + (o->data[6]*m->data[1]) + (o->data[10]*m->data[2]);
    result.data[6]  = (o->data[2]*m->data[4]) + (o->data[6]*m->data[5]) + (o->data[10]*m->data[6]);
    result.data[10] = (o->data[2]*m->data[8]) + (o->data[6]*m->data[9]) + (o->data[10]*m->data[10]);

    result.data[3]  = (o->data[3]*m->data[0]) + (o->data[7]*m->data[1]) +
                      (o->data[11]*m->data[2]) + m->data[3];
    result.data[7]  = (o->data[3]*m->data[4]) + (o->data[7]*m->data[5]) +
                      (o->data[11]*m->data[6]) + m->data[7];
    result.data[11] = (o->data[3]*m->data[8]) + (o->data[7]*m->data[9]) +
                      (o->data[11]*m->data[10]) + m->data[11];

    return result;
}

static inline cyclone_Vector3
cyclone_matrix4_mul_vector(const cyclone_Matrix4 *m,
                           const cyclone_Vector3 *v)
{
    cyclone_Vector3 r;
    r.x = v->x * m->data[0]  + v->y * m->data[1]  + v->z * m->data[2]  + m->data[3];
    r.y = v->x * m->data[4]  + v->y * m->data[5]  + v->z * m->data[6]  + m->data[7];
    r.z = v->x * m->data[8]  + v->y * m->data[9]  + v->z * m->data[10] + m->data[11];
    r.pad = (real)0;
    return r;
}

static inline cyclone_Vector3
cyclone_matrix4_transform(const cyclone_Matrix4 *m,
                          const cyclone_Vector3 *v)
{
    return cyclone_matrix4_mul_vector(m, v);
}

/* determinant / inverse are implemented in core.c */
real cyclone_matrix4_get_determinant(const cyclone_Matrix4 *m);
void cyclone_matrix4_set_inverse(cyclone_Matrix4 *out, const cyclone_Matrix4 *m);

static inline cyclone_Matrix4
cyclone_matrix4_inverse(const cyclone_Matrix4 *m)
{
    cyclone_Matrix4 result;
    cyclone_matrix4_set_inverse(&result, m);
    return result;
}

static inline void
cyclone_matrix4_invert(cyclone_Matrix4 *m)
{
    cyclone_Matrix4 tmp = *m;
    cyclone_matrix4_set_inverse(m, &tmp);
}

/* Direction transforms */

static inline cyclone_Vector3
cyclone_matrix4_transform_direction(const cyclone_Matrix4 *m,
                                    const cyclone_Vector3 *v)
{
    cyclone_Vector3 r;
    r.x = v->x * m->data[0]  + v->y * m->data[1]  + v->z * m->data[2];
    r.y = v->x * m->data[4]  + v->y * m->data[5]  + v->z * m->data[6];
    r.z = v->x * m->data[8]  + v->y * m->data[9]  + v->z * m->data[10];
    r.pad = (real)0;
    return r;
}

static inline cyclone_Vector3
cyclone_matrix4_transform_inverse_direction(const cyclone_Matrix4 *m,
                                            const cyclone_Vector3 *v)
{
    cyclone_Vector3 r;
    r.x = v->x * m->data[0]  + v->y * m->data[4]  + v->z * m->data[8];
    r.y = v->x * m->data[1]  + v->y * m->data[5]  + v->z * m->data[9];
    r.z = v->x * m->data[2]  + v->y * m->data[6]  + v->z * m->data[10];
    r.pad = (real)0;
    return r;
}

static inline cyclone_Vector3
cyclone_matrix4_transform_inverse(const cyclone_Matrix4 *m,
                                  const cyclone_Vector3 *v)
{
    cyclone_Vector3 tmp = {
        .x = v->x - m->data[3],
        .y = v->y - m->data[7],
        .z = v->z - m->data[11],
        .pad = 0
    };

    cyclone_Vector3 r;
    r.x = tmp.x * m->data[0]  + tmp.y * m->data[4]  + tmp.z * m->data[8];
    r.y = tmp.x * m->data[1]  + tmp.y * m->data[5]  + tmp.z * m->data[9];
    r.z = tmp.x * m->data[2]  + tmp.y * m->data[6]  + tmp.z * m->data[10];
    r.pad = (real)0;
    return r;
}

static inline cyclone_Vector3
cyclone_matrix4_get_axis_vector(const cyclone_Matrix4 *m, int i)
{
    cyclone_Vector3 r;
    r.x = m->data[i];
    r.y = m->data[i + 4];
    r.z = m->data[i + 8];
    r.pad = (real)0;
    return r;
}

static inline void
cyclone_matrix4_set_orientation_and_pos(cyclone_Matrix4 *m,
                                        const cyclone_Quaternion *q,
                                        const cyclone_Vector3 *pos)
{
    m->data[0]  = 1 - (2*q->j*q->j + 2*q->k*q->k);
    m->data[1]  = 2*q->i*q->j + 2*q->k*q->r;
    m->data[2]  = 2*q->i*q->k - 2*q->j*q->r;
    m->data[3]  = pos->x;

    m->data[4]  = 2*q->i*q->j - 2*q->k*q->r;
    m->data[5]  = 1 - (2*q->i*q->i  + 2*q->k*q->k);
    m->data[6]  = 2*q->j*q->k + 2*q->i*q->r;
    m->data[7]  = pos->y;

    m->data[8]  = 2*q->i*q->k + 2*q->j*q->r;
    m->data[9]  = 2*q->j*q->k - 2*q->i*q->r;
    m->data[10] = 1 - (2*q->i*q->i  + 2*q->j*q->j);
    m->data[11] = pos->z;
}

static inline void
cyclone_matrix4_fill_gl_array(const cyclone_Matrix4 *m, float array[16])
{
    array[0]  = (float)m->data[0];
    array[1]  = (float)m->data[4];
    array[2]  = (float)m->data[8];
    array[3]  = (float)0;

    array[4]  = (float)m->data[1];
    array[5]  = (float)m->data[5];
    array[6]  = (float)m->data[9];
    array[7]  = (float)0;

    array[8]  = (float)m->data[2];
    array[9]  = (float)m->data[6];
    array[10] = (float)m->data[10];
    array[11] = (float)0;

    array[12] = (float)m->data[3];
    array[13] = (float)m->data[7];
    array[14] = (float)m->data[11];
    array[15] = (float)1;
}

/* ============================================================
 * Matrix3 – 3x3 inertia / rotation matrix
 * ============================================================
 */

typedef struct cyclone_Matrix3 {
    real data[9];
} cyclone_Matrix3;

typedef cyclone_Matrix3 Matrix3;

static inline cyclone_Matrix3
cyclone_matrix3_zero(void)
{
    cyclone_Matrix3 m = { .data = {0} };
    return m;
}

static inline cyclone_Matrix3
cyclone_matrix3_make(real c0, real c1, real c2,
                     real c3, real c4, real c5,
                     real c6, real c7, real c8)
{
    cyclone_Matrix3 m = {
        .data = { c0, c1, c2, c3, c4, c5, c6, c7, c8 }
    };
    return m;
}

static inline cyclone_Matrix3
cyclone_matrix3_from_components(const cyclone_Vector3 *c1,
                                const cyclone_Vector3 *c2,
                                const cyclone_Vector3 *c3)
{
    cyclone_Matrix3 m;
    m.data[0] = c1->x; m.data[1] = c2->x; m.data[2] = c3->x;
    m.data[3] = c1->y; m.data[4] = c2->y; m.data[5] = c3->y;
    m.data[6] = c1->z; m.data[7] = c2->z; m.data[8] = c3->z;
    return m;
}

static inline void
cyclone_matrix3_set_diagonal(cyclone_Matrix3 *m, real a, real b, real c)
{
    m->data[0] = a;   m->data[1] = 0;   m->data[2] = 0;
    m->data[3] = 0;   m->data[4] = b;   m->data[5] = 0;
    m->data[6] = 0;   m->data[7] = 0;   m->data[8] = c;
}

static inline void
cyclone_matrix3_set_inertia_tensor_coeffs(cyclone_Matrix3 *m,
                                          real ix, real iy, real iz,
                                          real ixy, real ixz, real iyz)
{
    m->data[0] = ix;
    m->data[1] = m->data[3] = -ixy;
    m->data[2] = m->data[6] = -ixz;
    m->data[4] = iy;
    m->data[5] = m->data[7] = -iyz;
    m->data[8] = iz;
}

static inline void
cyclone_matrix3_set_block_inertia_tensor(cyclone_Matrix3 *m,
                                         const cyclone_Vector3 *half_sizes,
                                         real mass)
{
    cyclone_Vector3 squares = cyclone_vector3_component_product(*half_sizes, *half_sizes);
    real factor = (real)0.3f * mass;

    cyclone_matrix3_set_inertia_tensor_coeffs(
        m,
        factor * (squares.y + squares.z),
        factor * (squares.x + squares.z),
        factor * (squares.x + squares.y),
        0, 0, 0
    );
}

static inline void
cyclone_matrix3_set_skew_symmetric(cyclone_Matrix3 *m, cyclone_Vector3 v)
{
    m->data[0] = 0;
    m->data[4] = 0;
    m->data[8] = 0;

    m->data[1] = -v.z;
    m->data[2] =  v.y;
    m->data[3] =  v.z;
    m->data[5] = -v.x;
    m->data[6] = -v.y;
    m->data[7] =  v.x;
}

static inline cyclone_Vector3
cyclone_matrix3_mul_vector(const cyclone_Matrix3 *m,
                           const cyclone_Vector3 *v)
{
    cyclone_Vector3 r;
    r.x = v->x * m->data[0] + v->y * m->data[1] + v->z * m->data[2];
    r.y = v->x * m->data[3] + v->y * m->data[4] + v->z * m->data[5];
    r.z = v->x * m->data[6] + v->y * m->data[7] + v->z * m->data[8];
    r.pad = 0;
    return r;
}

static inline cyclone_Vector3
cyclone_matrix3_transform(const cyclone_Matrix3 *m,
                          const cyclone_Vector3 *v)
{
    return cyclone_matrix3_mul_vector(m, v);
}

static inline cyclone_Vector3
cyclone_matrix3_transform_transpose(const cyclone_Matrix3 *m,
                                    const cyclone_Vector3 *v)
{
    cyclone_Vector3 r;
    r.x = v->x * m->data[0] + v->y * m->data[3] + v->z * m->data[6];
    r.y = v->x * m->data[1] + v->y * m->data[4] + v->z * m->data[7];
    r.z = v->x * m->data[2] + v->y * m->data[5] + v->z * m->data[8];
    r.pad = 0;
    return r;
}

static inline cyclone_Vector3
cyclone_matrix3_get_row_vector(const cyclone_Matrix3 *m, int i)
{
    cyclone_Vector3 r;
    r.x = m->data[i*3 + 0];
    r.y = m->data[i*3 + 1];
    r.z = m->data[i*3 + 2];
    r.pad = 0;
    return r;
}

static inline cyclone_Vector3
cyclone_matrix3_get_axis_vector(const cyclone_Matrix3 *m, int i)
{
    cyclone_Vector3 r;
    r.x = m->data[i];
    r.y = m->data[i + 3];
    r.z = m->data[i + 6];
    r.pad = 0;
    return r;
}

static inline void
cyclone_matrix3_set_inverse(cyclone_Matrix3 *m, const cyclone_Matrix3 *src)
{
    real t4  = src->data[0]*src->data[4];
    real t6  = src->data[0]*src->data[5];
    real t8  = src->data[1]*src->data[3];
    real t10 = src->data[2]*src->data[3];
    real t12 = src->data[1]*src->data[6];
    real t14 = src->data[2]*src->data[6];

    real t16 = (t4 * src->data[8] - t6 * src->data[7] - t8 * src->data[8] +
                t10 * src->data[7] + t12 * src->data[5] - t14 * src->data[4]);

    if (t16 == (real)0.0f) return;
    real t17 = ((real)1) / t16;

    m->data[0] =  (src->data[4]*src->data[8] - src->data[5]*src->data[7]) * t17;
    m->data[1] = -(src->data[1]*src->data[8] - src->data[2]*src->data[7]) * t17;
    m->data[2] =  (src->data[1]*src->data[5] - src->data[2]*src->data[4]) * t17;
    m->data[3] = -(src->data[3]*src->data[8] - src->data[5]*src->data[6]) * t17;
    m->data[4] =  (src->data[0]*src->data[8] - t14) * t17;
    m->data[5] = -(t6 - t10) * t17;
    m->data[6] =  (src->data[3]*src->data[7] - src->data[4]*src->data[6]) * t17;
    m->data[7] = -(src->data[0]*src->data[7] - t12) * t17;
    m->data[8] =  (t4 - t8) * t17;
}

static inline cyclone_Matrix3
cyclone_matrix3_inverse(const cyclone_Matrix3 *m)
{
    cyclone_Matrix3 r;
    cyclone_matrix3_set_inverse(&r, m);
    return r;
}

static inline void
cyclone_matrix3_invert(cyclone_Matrix3 *m)
{
    cyclone_Matrix3 tmp = *m;
    cyclone_matrix3_set_inverse(m, &tmp);
}

static inline void
cyclone_matrix3_set_transpose(cyclone_Matrix3 *m, const cyclone_Matrix3 *src)
{
    m->data[0] = src->data[0];
    m->data[1] = src->data[3];
    m->data[2] = src->data[6];
    m->data[3] = src->data[1];
    m->data[4] = src->data[4];
    m->data[5] = src->data[7];
    m->data[6] = src->data[2];
    m->data[7] = src->data[5];
    m->data[8] = src->data[8];
}

static inline cyclone_Matrix3
cyclone_matrix3_transpose(const cyclone_Matrix3 *m)
{
    cyclone_Matrix3 r;
    cyclone_matrix3_set_transpose(&r, m);
    return r;
}

static inline cyclone_Matrix3
cyclone_matrix3_mul(const cyclone_Matrix3 *a, const cyclone_Matrix3 *b)
{
    cyclone_Matrix3 r;

    r.data[0] = a->data[0]*b->data[0] + a->data[1]*b->data[3] + a->data[2]*b->data[6];
    r.data[1] = a->data[0]*b->data[1] + a->data[1]*b->data[4] + a->data[2]*b->data[7];
    r.data[2] = a->data[0]*b->data[2] + a->data[1]*b->data[5] + a->data[2]*b->data[8];

    r.data[3] = a->data[3]*b->data[0] + a->data[4]*b->data[3] + a->data[5]*b->data[6];
    r.data[4] = a->data[3]*b->data[1] + a->data[4]*b->data[4] + a->data[5]*b->data[7];
    r.data[5] = a->data[3]*b->data[2] + a->data[4]*b->data[5] + a->data[5]*b->data[8];

    r.data[6] = a->data[6]*b->data[0] + a->data[7]*b->data[3] + a->data[8]*b->data[6];
    r.data[7] = a->data[6]*b->data[1] + a->data[7]*b->data[4] + a->data[8]*b->data[7];
    r.data[8] = a->data[6]*b->data[2] + a->data[7]*b->data[5] + a->data[8]*b->data[8];

    return r;
}

static inline void
cyclone_matrix3_mul_inplace(cyclone_Matrix3 *m, const cyclone_Matrix3 *o)
{
    *m = cyclone_matrix3_mul(m, o);
}

static inline void
cyclone_matrix3_scale_inplace(cyclone_Matrix3 *m, real s)
{
    for (int i = 0; i < 9; ++i)
        m->data[i] *= s;
}

static inline void
cyclone_matrix3_add_inplace(cyclone_Matrix3 *m, const cyclone_Matrix3 *o)
{
    for (int i = 0; i < 9; ++i)
        m->data[i] += o->data[i];
}

static inline void
cyclone_matrix3_set_orientation(cyclone_Matrix3 *m, const cyclone_Quaternion *q)
{
    m->data[0] = 1 - (2*q->j*q->j + 2*q->k*q->k);
    m->data[1] = 2*q->i*q->j + 2*q->k*q->r;
    m->data[2] = 2*q->i*q->k - 2*q->j*q->r;
    m->data[3] = 2*q->i*q->j - 2*q->k*q->r;
    m->data[4] = 1 - (2*q->i*q->i  + 2*q->k*q->k);
    m->data[5] = 2*q->j*q->k + 2*q->i*q->r;
    m->data[6] = 2*q->i*q->k + 2*q->j*q->r;
    m->data[7] = 2*q->j*q->k - 2*q->i*q->r;
    m->data[8] = 1 - (2*q->i*q->i  + 2*q->j*q->j);
}

static inline cyclone_Matrix3
cyclone_matrix3_linear_interpolate(const cyclone_Matrix3 *a,
                                   const cyclone_Matrix3 *b,
                                   real prop)
{
    cyclone_Matrix3 r;
    for (int i = 0; i < 9; ++i)
    {
        r.data[i] = a->data[i] * ((real)1 - prop) + b->data[i] * prop;
    }
    return r;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CYCLONE_CORE_H */
