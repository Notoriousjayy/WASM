#include "geometry2d.h"
#include "matrices.h"

#include <math.h>
#include <float.h>

#ifdef DO_SANITY_TESTS
    #include "compare.h"
#else
    #define CMP(x, y) \
        (fabsf((x) - (y)) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))
#endif

#ifndef DEG2RAD
    #define DEG2RAD(deg) ((deg) * 0.0174532925199433f)  // deg * (PI / 180)
#endif

#define CLAMP(number, minimum, maximum) \
    ((number) = ((number) < (minimum)) ? (minimum) : \
        (((number) > (maximum)) ? (maximum) : (number)))

#define OVERLAP(minA, maxA, minB, maxB) \
    (((minB) <= (maxA)) && ((minA) <= (maxB)))

/*******************************************************************************
 * Line2D Operations
 ******************************************************************************/

float line2d_length(Line2D line) {
    return vec2_magnitude(vec2_sub(line.end, line.start));
}

float line2d_length_sq(Line2D line) {
    return vec2_magnitude_sq(vec2_sub(line.end, line.start));
}

/*******************************************************************************
 * Rectangle2D Operations
 ******************************************************************************/

vec2 rectangle2d_get_min(Rectangle2D rect) {
    vec2 p1 = rect.origin;
    vec2 p2 = vec2_add(rect.origin, rect.size);
    return (vec2){ fminf(p1.x, p2.x), fminf(p1.y, p2.y) };
}

vec2 rectangle2d_get_max(Rectangle2D rect) {
    vec2 p1 = rect.origin;
    vec2 p2 = vec2_add(rect.origin, rect.size);
    return (vec2){ fmaxf(p1.x, p2.x), fmaxf(p1.y, p2.y) };
}

Rectangle2D rectangle2d_from_min_max(vec2 min, vec2 max) {
    return rectangle2d_create(min, vec2_sub(max, min));
}

/*******************************************************************************
 * 2x2 Rotation Helper (internal)
 ******************************************************************************/

static vec2 rotate_vec2(vec2 v, float theta_rad) {
    float c = cosf(theta_rad);
    float s = sinf(theta_rad);
    return (vec2){
        v.x * c + v.y * (-s),
        v.x * s + v.y * c
    };
}

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_on_line2d(Point2D point, Line2D line) {
    // Find the slope
    float dx = line.end.x - line.start.x;
    float dy = line.end.y - line.start.y;
    
    // Handle vertical line
    if (CMP(dx, 0.0f)) {
        return CMP(point.x, line.start.x);
    }
    
    float M = dy / dx;
    float B = line.start.y - M * line.start.x;
    return CMP(point.y, M * point.x + B);
}

bool point_in_circle(Point2D point, Circle circle) {
    Line2D line = line2d_create(point, circle.position);
    float dist_sq = line2d_length_sq(line);
    return dist_sq < circle.radius * circle.radius;
}

bool point_in_rectangle2d(Point2D point, Rectangle2D rect) {
    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    return min.x <= point.x && min.y <= point.y &&
           point.x <= max.x && point.y <= max.y;
}

bool point_in_oriented_rectangle(Point2D point, OrientedRectangle rect) {
    // Create local-space rectangle (origin at 0,0, size = 2 * half_extents)
    Rectangle2D local_rect = rectangle2d_create(
        (Point2D){0, 0}, 
        vec2_scale(rect.half_extents, 2.0f)
    );

    // Get vector from rectangle center to point
    vec2 rot_vector = vec2_sub(point, rect.position);

    // Rotate by inverse of rectangle's rotation
    float theta = -DEG2RAD(rect.rotation);
    rot_vector = rotate_vec2(rot_vector, theta);

    // Offset to account for Rectangle2D having origin at corner
    vec2 local_point = vec2_add(rot_vector, rect.half_extents);

    return point_in_rectangle2d(local_point, local_rect);
}

/*******************************************************************************
 * Line Intersection Tests
 ******************************************************************************/

bool line2d_circle(Line2D line, Circle circle) {
    vec2 ab = vec2_sub(line.end, line.start);

    // Project circle position onto line segment
    float t = vec2_dot(vec2_sub(circle.position, line.start), ab) / vec2_dot(ab, ab);

    // Check if closest point is within segment
    if (t < 0.0f || t > 1.0f) {
        return false;
    }

    // Find closest point on segment
    Point2D closest = vec2_add(line.start, vec2_scale(ab, t));

    Line2D to_closest = line2d_create(circle.position, closest);
    return line2d_length_sq(to_closest) < circle.radius * circle.radius;
}

bool line2d_rectangle(Line2D line, Rectangle2D rect) {
    // Quick check: either endpoint inside rectangle
    if (point_in_rectangle2d(line.start, rect) || 
        point_in_rectangle2d(line.end, rect)) {
        return true;
    }

    vec2 dir = vec2_sub(line.end, line.start);
    vec2 norm = vec2_normalized(dir);
    
    // Avoid division by zero
    norm.x = (norm.x != 0) ? 1.0f / norm.x : 0;
    norm.y = (norm.y != 0) ? 1.0f / norm.y : 0;

    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    vec2 t_min = vec2_mul(vec2_sub(min, line.start), norm);
    vec2 t_max = vec2_mul(vec2_sub(max, line.start), norm);

    float tmin = fmaxf(fminf(t_min.x, t_max.x), fminf(t_min.y, t_max.y));
    float tmax = fminf(fmaxf(t_min.x, t_max.x), fmaxf(t_min.y, t_max.y));

    if (tmax < 0 || tmin > tmax) {
        return false;
    }

    float t = (tmin < 0.0f) ? tmax : tmin;
    return t > 0.0f && t * t < line2d_length_sq(line);
}

bool line2d_oriented_rectangle(Line2D line, OrientedRectangle rect) {
    float theta = -DEG2RAD(rect.rotation);

    // Transform line endpoints to local space
    vec2 rot_start = vec2_sub(line.start, rect.position);
    rot_start = rotate_vec2(rot_start, theta);
    
    vec2 rot_end = vec2_sub(line.end, rect.position);
    rot_end = rotate_vec2(rot_end, theta);

    Line2D local_line = {
        .start = vec2_add(rot_start, rect.half_extents),
        .end = vec2_add(rot_end, rect.half_extents)
    };

    Rectangle2D local_rect = rectangle2d_create(
        (Point2D){0, 0},
        vec2_scale(rect.half_extents, 2.0f)
    );

    return line2d_rectangle(local_line, local_rect);
}

/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool circle_circle(Circle c1, Circle c2) {
    Line2D line = line2d_create(c1.position, c2.position);
    float radii_sum = c1.radius + c2.radius;
    return line2d_length_sq(line) <= radii_sum * radii_sum;
}

bool circle_rectangle(Circle circle, Rectangle2D rect) {
    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    // Find closest point on rectangle to circle center
    Point2D closest = circle.position;
    closest.x = (closest.x < min.x) ? min.x : 
                (closest.x > max.x) ? max.x : closest.x;
    closest.y = (closest.y < min.y) ? min.y : 
                (closest.y > max.y) ? max.y : closest.y;

    Line2D line = line2d_create(circle.position, closest);
    return line2d_length_sq(line) <= circle.radius * circle.radius;
}

bool circle_oriented_rectangle(Circle circle, OrientedRectangle rect) {
    float theta = -DEG2RAD(rect.rotation);

    // Transform circle to local space
    vec2 rot_pos = vec2_sub(circle.position, rect.position);
    rot_pos = rotate_vec2(rot_pos, theta);

    Circle local_circle = circle_create(
        vec2_add(rot_pos, rect.half_extents),
        circle.radius
    );

    Rectangle2D local_rect = rectangle2d_create(
        (Point2D){0, 0},
        vec2_scale(rect.half_extents, 2.0f)
    );

    return circle_rectangle(local_circle, local_rect);
}

bool rectangle_rectangle(Rectangle2D r1, Rectangle2D r2) {
    vec2 a_min = rectangle2d_get_min(r1);
    vec2 a_max = rectangle2d_get_max(r1);
    vec2 b_min = rectangle2d_get_min(r2);
    vec2 b_max = rectangle2d_get_max(r2);

    bool x_overlap = (b_min.x <= a_max.x) && (a_min.x <= b_max.x);
    bool y_overlap = (b_min.y <= a_max.y) && (a_min.y <= b_max.y);

    return x_overlap && y_overlap;
}

/*******************************************************************************
 * SAT Functions
 ******************************************************************************/

Interval2D interval2d_from_rectangle(Rectangle2D rect, vec2 axis) {
    vec2 min = rectangle2d_get_min(rect);
    vec2 max = rectangle2d_get_max(rect);

    vec2 verts[4] = {
        { min.x, min.y },
        { min.x, max.y },
        { max.x, max.y },
        { max.x, min.y }
    };

    Interval2D result;
    result.min = vec2_dot(axis, verts[0]);
    result.max = result.min;

    for (int i = 1; i < 4; ++i) {
        float proj = vec2_dot(axis, verts[i]);
        result.min = (proj < result.min) ? proj : result.min;
        result.max = (proj > result.max) ? proj : result.max;
    }

    return result;
}

Interval2D interval2d_from_oriented_rectangle(OrientedRectangle rect, vec2 axis) {
    // Create non-oriented rect centered at position
    vec2 min = vec2_sub(rect.position, rect.half_extents);
    vec2 max = vec2_add(rect.position, rect.half_extents);

    vec2 verts[4] = {
        min,
        max,
        { min.x, max.y },
        { max.x, min.y }
    };

    float theta = DEG2RAD(rect.rotation);

    // Rotate vertices around center
    for (int i = 0; i < 4; ++i) {
        vec2 rel = vec2_sub(verts[i], rect.position);
        rel = rotate_vec2(rel, theta);
        verts[i] = vec2_add(rel, rect.position);
    }

    Interval2D result;
    result.min = result.max = vec2_dot(axis, verts[0]);

    for (int i = 1; i < 4; ++i) {
        float proj = vec2_dot(axis, verts[i]);
        result.min = (proj < result.min) ? proj : result.min;
        result.max = (proj > result.max) ? proj : result.max;
    }

    return result;
}

bool overlap_on_axis_rect_rect(Rectangle2D r1, Rectangle2D r2, vec2 axis) {
    Interval2D a = interval2d_from_rectangle(r1, axis);
    Interval2D b = interval2d_from_rectangle(r2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_rect_oriented(Rectangle2D r1, OrientedRectangle r2, vec2 axis) {
    Interval2D a = interval2d_from_rectangle(r1, axis);
    Interval2D b = interval2d_from_oriented_rectangle(r2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_oriented_oriented(OrientedRectangle r1, OrientedRectangle r2, vec2 axis) {
    Interval2D a = interval2d_from_oriented_rectangle(r1, axis);
    Interval2D b = interval2d_from_oriented_rectangle(r2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool rectangle_rectangle_sat(Rectangle2D r1, Rectangle2D r2) {
    vec2 axes[2] = {
        { 1, 0 },
        { 0, 1 }
    };

    for (int i = 0; i < 2; ++i) {
        if (!overlap_on_axis_rect_rect(r1, r2, axes[i])) {
            return false;
        }
    }

    return true;
}

bool rectangle_oriented_rectangle(Rectangle2D r1, OrientedRectangle r2) {
    float theta = DEG2RAD(r2.rotation);

    // Axes to test: 2 from AABB + 2 from oriented rectangle
    vec2 axes[4] = {
        { 1, 0 },
        { 0, 1 },
        rotate_vec2((vec2){1, 0}, theta),
        rotate_vec2((vec2){0, 1}, theta)
    };

    for (int i = 0; i < 4; ++i) {
        if (!overlap_on_axis_rect_oriented(r1, r2, axes[i])) {
            return false;
        }
    }

    return true;
}

bool oriented_rectangle_oriented_rectangle_sat(OrientedRectangle r1, OrientedRectangle r2) {
    float theta1 = DEG2RAD(r1.rotation);
    float theta2 = DEG2RAD(r2.rotation);

    vec2 axes[6] = {
        { 1, 0 },
        { 0, 1 },
        rotate_vec2((vec2){1, 0}, theta2),
        rotate_vec2((vec2){0, 1}, theta2),
        rotate_vec2((vec2){1, 0}, theta1),
        rotate_vec2((vec2){0, 1}, theta1)
    };

    for (int i = 0; i < 6; ++i) {
        if (!overlap_on_axis_oriented_oriented(r1, r2, axes[i])) {
            return false;
        }
    }

    return true;
}

bool oriented_rectangle_oriented_rectangle(OrientedRectangle r1, OrientedRectangle r2) {
    // Transform r2 into r1's local space
    Rectangle2D local_r1 = rectangle2d_create(
        (Point2D){0, 0},
        vec2_scale(r1.half_extents, 2.0f)
    );

    OrientedRectangle local_r2 = r2;
    local_r2.rotation = r2.rotation - r1.rotation;

    float theta = -DEG2RAD(r1.rotation);
    vec2 rot_pos = vec2_sub(r2.position, r1.position);
    rot_pos = rotate_vec2(rot_pos, theta);
    local_r2.position = vec2_add(rot_pos, r1.half_extents);

    return rectangle_oriented_rectangle(local_r1, local_r2);
}

/*******************************************************************************
 * Bounding Volume Generation
 ******************************************************************************/

Circle containing_circle(Point2D* points, int count) {
    if (count <= 0) {
        return circle_default();
    }

    // Compute centroid
    Point2D center = {0, 0};
    for (int i = 0; i < count; ++i) {
        center = vec2_add(center, points[i]);
    }
    center = vec2_scale(center, 1.0f / (float)count);

    // Find maximum squared distance from center
    float radius_sq = vec2_magnitude_sq(vec2_sub(center, points[0]));
    for (int i = 1; i < count; ++i) {
        float dist_sq = vec2_magnitude_sq(vec2_sub(center, points[i]));
        if (dist_sq > radius_sq) {
            radius_sq = dist_sq;
        }
    }

    return circle_create(center, sqrtf(radius_sq));
}

#ifndef NO_EXTRAS
Circle containing_circle_alt(Point2D* points, int count) {
    if (count <= 0) {
        return circle_default();
    }

    vec2 min = points[0];
    vec2 max = points[0];

    for (int i = 1; i < count; ++i) {
        min.x = (points[i].x < min.x) ? points[i].x : min.x;
        min.y = (points[i].y < min.y) ? points[i].y : min.y;
        max.x = (points[i].x > max.x) ? points[i].x : max.x;
        max.y = (points[i].y > max.y) ? points[i].y : max.y;
    }

    vec2 center = vec2_scale(vec2_add(min, max), 0.5f);
    float radius = vec2_magnitude(vec2_sub(max, min)) * 0.5f;

    return circle_create(center, radius);
}
#endif

Rectangle2D containing_rectangle(Point2D* points, int count) {
    if (count <= 0) {
        return rectangle2d_default();
    }

    vec2 min = points[0];
    vec2 max = points[0];

    for (int i = 1; i < count; ++i) {
        min.x = (points[i].x < min.x) ? points[i].x : min.x;
        min.y = (points[i].y < min.y) ? points[i].y : min.y;
        max.x = (points[i].x > max.x) ? points[i].x : max.x;
        max.y = (points[i].y > max.y) ? points[i].y : max.y;
    }

    return rectangle2d_from_min_max(min, max);
}

/*******************************************************************************
 * BoundingShape Tests
 ******************************************************************************/

bool point_in_bounding_shape(BoundingShape shape, Point2D point) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (point_in_circle(point, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (point_in_rectangle2d(point, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

#ifndef NO_EXTRAS
bool line2d_bounding_shape(Line2D line, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (line2d_circle(line, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (line2d_rectangle(line, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

bool circle_bounding_shape(Circle circle, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (circle_circle(circle, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (circle_rectangle(circle, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

bool rectangle_bounding_shape(Rectangle2D rect, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (rectangle_circle(rect, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (rectangle_rectangle(rect, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}

bool oriented_rectangle_bounding_shape(OrientedRectangle rect, BoundingShape shape) {
    for (int i = 0; i < shape.num_circles; ++i) {
        if (oriented_rectangle_circle(rect, shape.circles[i])) {
            return true;
        }
    }
    for (int i = 0; i < shape.num_rectangles; ++i) {
        if (oriented_rectangle_rectangle(rect, shape.rectangles[i])) {
            return true;
        }
    }
    return false;
}
#endif

/*******************************************************************************
 * Debug Print Functions
 ******************************************************************************/

#ifndef NO_EXTRAS
void line2d_print(FILE* stream, Line2D shape) {
    fprintf(stream, "start: (%.3f, %.3f), end: (%.3f, %.3f)",
            shape.start.x, shape.start.y,
            shape.end.x, shape.end.y);
}

void circle_print(FILE* stream, Circle shape) {
    fprintf(stream, "position: (%.3f, %.3f), radius: %.3f",
            shape.position.x, shape.position.y, shape.radius);
}

void rectangle2d_print(FILE* stream, Rectangle2D shape) {
    vec2 min = rectangle2d_get_min(shape);
    vec2 max = rectangle2d_get_max(shape);
    fprintf(stream, "min: (%.3f, %.3f), max: (%.3f, %.3f)",
            min.x, min.y, max.x, max.y);
}

void oriented_rectangle_print(FILE* stream, OrientedRectangle shape) {
    fprintf(stream, "position: (%.3f, %.3f), half size: (%.3f, %.3f), rotation: %.3f",
            shape.position.x, shape.position.y,
            shape.half_extents.x, shape.half_extents.y,
            shape.rotation);
}
#endif