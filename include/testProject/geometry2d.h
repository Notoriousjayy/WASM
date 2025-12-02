#ifndef GEOMETRY2D_H
#define GEOMETRY2D_H

#include "vectors.h"
#include <stdbool.h>
#include <stdio.h>

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef vec2 Point2D;

typedef struct Line2D {
    Point2D start;
    Point2D end;
} Line2D;

typedef struct Circle {
    Point2D position;
    float   radius;
} Circle;

typedef struct Rectangle2D {
    Point2D origin;
    vec2    size;
} Rectangle2D;

typedef struct OrientedRectangle {
    Point2D position;
    vec2    half_extents;
    float   rotation;       // Degrees
} OrientedRectangle;

typedef struct BoundingShape {
    int          num_circles;
    Circle*      circles;
    int          num_rectangles;
    Rectangle2D* rectangles;
} BoundingShape;

typedef struct Interval2D {
    float min;
    float max;
} Interval2D;

/*******************************************************************************
 * Constructors / Initializers
 ******************************************************************************/

static inline Line2D line2d_create(Point2D start, Point2D end) {
    return (Line2D){ .start = start, .end = end };
}

static inline Line2D line2d_default(void) {
    return (Line2D){ .start = {0, 0}, .end = {0, 0} };
}

static inline Circle circle_create(Point2D position, float radius) {
    return (Circle){ .position = position, .radius = radius };
}

static inline Circle circle_default(void) {
    return (Circle){ .position = {0, 0}, .radius = 1.0f };
}

static inline Rectangle2D rectangle2d_create(Point2D origin, vec2 size) {
    return (Rectangle2D){ .origin = origin, .size = size };
}

static inline Rectangle2D rectangle2d_default(void) {
    return (Rectangle2D){ .origin = {0, 0}, .size = {1, 1} };
}

static inline OrientedRectangle oriented_rectangle_create(Point2D pos, vec2 half_extents, float rotation) {
    return (OrientedRectangle){ 
        .position = pos, 
        .half_extents = half_extents, 
        .rotation = rotation 
    };
}

static inline OrientedRectangle oriented_rectangle_default(void) {
    return (OrientedRectangle){ 
        .position = {0, 0}, 
        .half_extents = {1.0f, 1.0f}, 
        .rotation = 0.0f 
    };
}

static inline BoundingShape bounding_shape_default(void) {
    return (BoundingShape){
        .num_circles = 0,
        .circles = NULL,
        .num_rectangles = 0,
        .rectangles = NULL
    };
}

/*******************************************************************************
 * Line2D Operations
 ******************************************************************************/

float line2d_length(Line2D line);
float line2d_length_sq(Line2D line);

/*******************************************************************************
 * Rectangle2D Operations
 ******************************************************************************/

vec2        rectangle2d_get_min(Rectangle2D rect);
vec2        rectangle2d_get_max(Rectangle2D rect);
Rectangle2D rectangle2d_from_min_max(vec2 min, vec2 max);

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_on_line2d(Point2D point, Line2D line);
bool point_in_circle(Point2D point, Circle circle);
bool point_in_rectangle2d(Point2D point, Rectangle2D rect);
bool point_in_oriented_rectangle(Point2D point, OrientedRectangle rect);

/*******************************************************************************
 * Line Intersection Tests
 ******************************************************************************/

bool line2d_circle(Line2D line, Circle circle);
bool line2d_rectangle(Line2D line, Rectangle2D rect);
bool line2d_oriented_rectangle(Line2D line, OrientedRectangle rect);

/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool circle_circle(Circle c1, Circle c2);
bool circle_rectangle(Circle circle, Rectangle2D rect);
bool circle_oriented_rectangle(Circle circle, OrientedRectangle rect);
bool rectangle_rectangle(Rectangle2D r1, Rectangle2D r2);
bool rectangle_oriented_rectangle(Rectangle2D rect, OrientedRectangle oriented);
bool oriented_rectangle_oriented_rectangle(OrientedRectangle r1, OrientedRectangle r2);

/*******************************************************************************
 * SAT (Separating Axis Theorem) Functions
 ******************************************************************************/

Interval2D interval2d_from_rectangle(Rectangle2D rect, vec2 axis);
Interval2D interval2d_from_oriented_rectangle(OrientedRectangle rect, vec2 axis);

bool overlap_on_axis_rect_rect(Rectangle2D r1, Rectangle2D r2, vec2 axis);
bool overlap_on_axis_rect_oriented(Rectangle2D r1, OrientedRectangle r2, vec2 axis);
bool overlap_on_axis_oriented_oriented(OrientedRectangle r1, OrientedRectangle r2, vec2 axis);

bool rectangle_rectangle_sat(Rectangle2D r1, Rectangle2D r2);
bool oriented_rectangle_oriented_rectangle_sat(OrientedRectangle r1, OrientedRectangle r2);

/*******************************************************************************
 * Bounding Volume Generation
 ******************************************************************************/

Circle      containing_circle(Point2D* points, int count);
Rectangle2D containing_rectangle(Point2D* points, int count);

#ifndef NO_EXTRAS
Circle      containing_circle_alt(Point2D* points, int count);
#endif

/*******************************************************************************
 * BoundingShape Tests
 ******************************************************************************/

bool point_in_bounding_shape(BoundingShape shape, Point2D point);

#ifndef NO_EXTRAS
bool line2d_bounding_shape(Line2D line, BoundingShape shape);
bool circle_bounding_shape(Circle circle, BoundingShape shape);
bool rectangle_bounding_shape(Rectangle2D rect, BoundingShape shape);
bool oriented_rectangle_bounding_shape(OrientedRectangle rect, BoundingShape shape);
#endif

/*******************************************************************************
 * Convenience Macros (Argument Order Swapping)
 ******************************************************************************/

#define point_line2d(point, line)           point_on_line2d(point, line)
#define line2d_point(line, point)           point_on_line2d(point, line)
#define circle_line2d(circle, line)         line2d_circle(line, circle)
#define rectangle_line2d(rect, line)        line2d_rectangle(line, rect)
#define oriented_rectangle_line2d(r, line)  line2d_oriented_rectangle(line, r)

#ifndef NO_EXTRAS
#define point_circle(point, circle)              point_in_circle(point, circle)
#define circle_point(circle, point)              point_in_circle(point, circle)
#define point_rectangle(point, rect)             point_in_rectangle2d(point, rect)
#define rectangle_point(rect, point)             point_in_rectangle2d(point, rect)
#define point_oriented_rectangle(point, rect)    point_in_oriented_rectangle(point, rect)
#define oriented_rectangle_point(rect, point)    point_in_oriented_rectangle(point, rect)
#endif

#define rectangle_circle(rect, circle)           circle_rectangle(circle, rect)
#define oriented_rectangle_circle(rect, circle)  circle_oriented_rectangle(circle, rect)
#define oriented_rectangle_rectangle(o, r)       rectangle_oriented_rectangle(r, o)

#ifndef NO_EXTRAS
#define bounding_shape_line2d(shape, line)                  line2d_bounding_shape(line, shape)
#define bounding_shape_circle(shape, circle)                circle_bounding_shape(circle, shape)
#define bounding_shape_rectangle(shape, rect)               rectangle_bounding_shape(rect, shape)
#define bounding_shape_oriented_rectangle(shape, rect)      oriented_rectangle_bounding_shape(rect, shape)
#endif

/*******************************************************************************
 * Debug Print Functions (replaces C++ operator<<)
 ******************************************************************************/

#ifndef NO_EXTRAS
void line2d_print(FILE* stream, Line2D shape);
void circle_print(FILE* stream, Circle shape);
void rectangle2d_print(FILE* stream, Rectangle2D shape);
void oriented_rectangle_print(FILE* stream, OrientedRectangle shape);
#endif

#endif // GEOMETRY2D_H