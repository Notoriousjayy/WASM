#include "precision.h"

/*
 * Implementation file for precision-dependent helpers.
 *
 * Most of the configuration lives in the header via typedefs and macros.
 * This file provides:
 *   - the cyclone_pi constant
 *   - basic degree/radian conversion helpers
 */

const real cyclone_pi = (real)R_PI;

real real_deg_to_rad(real degrees)
{
    /* π / 180 */
    return degrees * (cyclone_pi / (real)180.0);
}

real real_rad_to_deg(real radians)
{
    /* 180 / π */
    return radians * ((real)180.0 / cyclone_pi);
}
