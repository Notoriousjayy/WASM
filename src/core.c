#include "core.h"

/* ============================================================
 * Global configuration + constants
 * ============================================================
 */

real cyclone_sleep_epsilon = (real)0.1;

void cyclone_set_sleep_epsilon(real value)
{
    cyclone_sleep_epsilon = value;
}

real cyclone_get_sleep_epsilon(void)
{
    return cyclone_sleep_epsilon;
}

/* Vector constants */

const cyclone_Vector3 CYCLONE_VECTOR3_GRAVITY       = {  0.0f, -9.81f,  0.0f, 0.0f };
const cyclone_Vector3 CYCLONE_VECTOR3_HIGH_GRAVITY  = {  0.0f, -19.62f, 0.0f, 0.0f };
const cyclone_Vector3 CYCLONE_VECTOR3_UP            = {  0.0f,  1.0f,   0.0f, 0.0f };
const cyclone_Vector3 CYCLONE_VECTOR3_RIGHT         = {  1.0f,  0.0f,   0.0f, 0.0f };
const cyclone_Vector3 CYCLONE_VECTOR3_OUT_OF_SCREEN = {  0.0f,  0.0f,   1.0f, 0.0f };
const cyclone_Vector3 CYCLONE_VECTOR3_X             = {  1.0f,  0.0f,   0.0f, 0.0f };
const cyclone_Vector3 CYCLONE_VECTOR3_Y             = {  0.0f,  1.0f,   0.0f, 0.0f };
const cyclone_Vector3 CYCLONE_VECTOR3_Z             = {  0.0f,  0.0f,   1.0f, 0.0f };

/* ============================================================
 * Matrix4 determinant + inverse
 * ============================================================
 *
 * Matrix4 is a 3x4 row-major transform:
 *
 * [ 0  1  2  3 ]
 * [ 4  5  6  7 ]
 * [ 8  9 10 11 ]
 *
 * with implicit last row [0 0 0 1].
 * The determinant is just det of the 3x3 rotation/scale block.
 */

real cyclone_matrix4_get_determinant(const cyclone_Matrix4 *m)
{
    real a = m->data[0];
    real b = m->data[1];
    real c = m->data[2];
    real d = m->data[4];
    real e = m->data[5];
    real f = m->data[6];
    real g = m->data[8];
    real h = m->data[9];
    real i = m->data[10];

    /* Standard 3x3 determinant */
    return a*e*i + b*f*g + c*d*h
         - c*e*g - a*f*h - b*d*i;
}

void cyclone_matrix4_set_inverse(cyclone_Matrix4 *out, const cyclone_Matrix4 *m)
{
    /* Extract 3x3 rotation/scale block */
    real a = m->data[0];
    real b = m->data[1];
    real c = m->data[2];
    real d = m->data[4];
    real e = m->data[5];
    real f = m->data[6];
    real g = m->data[8];
    real h = m->data[9];
    real i = m->data[10];

    real det = a*e*i + b*f*g + c*d*h
             - c*e*g - a*f*h - b*d*i;

    if (det == (real)0)
    {
        /* Non-invertible; leave out unchanged */
        return;
    }

    real invDet = ((real)1) / det;

    /* Cofactors for the rotation part */
    real C00 =  e*i - f*h;
    real C01 =  f*g - d*i;
    real C02 =  d*h - e*g;

    real C10 =  c*h - b*i;
    real C11 =  a*i - c*g;
    real C12 =  b*g - a*h;

    real C20 =  b*f - c*e;
    real C21 =  c*d - a*f;
    real C22 =  a*e - b*d;

    /* adj(R) / det(R) -> R^{-1} */
    out->data[0]  = C00 * invDet;
    out->data[1]  = C10 * invDet;
    out->data[2]  = C20 * invDet;

    out->data[4]  = C01 * invDet;
    out->data[5]  = C11 * invDet;
    out->data[6]  = C21 * invDet;

    out->data[8]  = C02 * invDet;
    out->data[9]  = C12 * invDet;
    out->data[10] = C22 * invDet;

    /* Original translation */
    real tx = m->data[3];
    real ty = m->data[7];
    real tz = m->data[11];

    /* New translation is -R^{-1} * t */
    out->data[3]  = -(out->data[0]*tx + out->data[1]*ty + out->data[2]*tz);
    out->data[7]  = -(out->data[4]*tx + out->data[5]*ty + out->data[6]*tz);
    out->data[11] = -(out->data[8]*tx + out->data[9]*ty + out->data[10]*tz);
}
