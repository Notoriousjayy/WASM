#ifndef GEOMETRY2D_H
#define GEOMETRY2D_H

/*******************************************************************************
 * Master Header - Geometry2D
 * 
 * This header provides a single include point for the entire 2D geometry
 * library. For finer-grained control, include individual headers directly.
 ******************************************************************************/

/* Core types and constructors */
#include "geometry2d_types.h"

/* Basic operations */
#include "line2d.h"
#include "rectangle2d.h"

/* Point containment tests */
#include "point_containment.h"

/* Line intersection tests */
#include "line_intersection.h"

/* Shape-shape collision tests */
#include "shape_collision.h"

/* Separating Axis Theorem */
#include "sat2d.h"

/* Bounding volume generation */
#include "bounding_volume.h"

/* Composite bounding shape tests */
#include "bounding_shape.h"

/* Debug print functions */
#include "geometry2d_debug.h"

#endif // GEOMETRY2D_H