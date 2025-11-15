#ifndef POLYGON_H
#define POLYGON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

/**
 * A 2D point (x, y).
 */
typedef struct {
    double x;
    double y;
} Point2D;

/**
 * Polygon represented as a dynamic array of vertices.
 *
 * The vertices are assumed to be ordered and the polygon is treated as closed:
 * edge (i -> i+1) for all i, plus edge (last -> first).
 */
typedef struct {
    Point2D *vertices;
    size_t   count;
    size_t   capacity;
} Polygon;

/**
 * Initialize an empty polygon.
 */
void polygon_init(Polygon *poly);

/**
 * Initialize an empty polygon with a given initial capacity.
 * Returns false on allocation failure.
 */
bool polygon_init_with_capacity(Polygon *poly, size_t capacity);

/**
 * Free all memory used by the polygon and reset it to an empty state.
 */
void polygon_clear(Polygon *poly);

/**
 * Append a vertex (x, y) to the end of the polygon's vertex list.
 * Returns false on allocation failure.
 */
bool polygon_add_vertex(Polygon *poly, double x, double y);

/**
 * Insert a vertex (x, y) at the given index.
 *
 * Valid indices are 0..count (inclusive); index == count is equivalent to append.
 * Returns false on allocation failure or if index is out of range.
 */
bool polygon_insert_vertex(Polygon *poly, size_t index, double x, double y);

/**
 * Remove the vertex at the given index.
 * Returns false if index is out of range.
 */
bool polygon_remove_vertex(Polygon *poly, size_t index);

/**
 * Returns true if the polygon has at least 3 vertices.
 */
bool polygon_is_valid(const Polygon *poly);

/**
 * Translate (shift) the polygon by (dx, dy) in-place.
 */
void polygon_translate(Polygon *poly, double dx, double dy);

/**
 * Scale the polygon in-place by (sx, sy) about the origin (0, 0).
 */
void polygon_scale(Polygon *poly, double sx, double sy);

/**
 * Rotate the polygon in-place about the origin (0, 0) by the given angle in radians.
 * Positive angles rotate counterclockwise.
 */
void polygon_rotate(Polygon *poly, double radians);

/**
 * Compute the perimeter (sum of edge lengths) of the polygon.
 *
 * If the polygon has fewer than 2 vertices, the perimeter is 0.0.
 * The polygon is treated as closed (includes edge from last to first).
 */
double polygon_perimeter(const Polygon *poly);

/**
 * Populate the polygon with a regular N-gon centered at the origin.
 *
 * - Existing vertices are cleared.
 * - The polygon will contain 'sides' vertices equally spaced on a circle
 *   of radius 'radius' centered at (0, 0).
 *
 * Returns false if:
 *   - poly is NULL
 *   - sides < 3
 *   - radius <= 0
 *   - allocation fails
 */
bool polygon_make_regular_ngon(Polygon *poly, size_t sides, double radius);

/**
 * Copy polygon vertices into an interleaved float buffer:
 *
 *   buffer = [x0, y0, x1, y1, ..., x_{n-1}, y_{n-1}]
 *
 * Up to floor(buffer_capacity / 2) vertices are copied.
 *
 * Parameters:
 *   poly             - polygon to copy from
 *   buffer           - destination float array
 *   buffer_capacity  - number of float elements available in 'buffer'
 *
 * Returns:
 *   Number of vertices actually copied into 'buffer'.
 *
 * This is convenient for passing vertex data to graphics APIs that expect
 * 32-bit float positions (e.g., OpenGL, WebGL, Direct3D).
 */
size_t polygon_copy_to_float_xy(const Polygon *poly,
                                float        *buffer,
                                size_t        buffer_capacity);

/**
 * Allocate an interleaved float buffer for the polygon and copy vertices:
 *
 *   result = [x0, y0, x1, y1, ..., x_{n-1}, y_{n-1}]
 *
 * Parameters:
 *   poly          - polygon to copy from
 *   out_float_cnt - optional; if non-NULL, receives the number of floats
 *                   stored in the returned buffer (2 * poly->count).
 *
 * Returns:
 *   - Pointer to a heap-allocated float array on success.
 *   - NULL on allocation failure or if the polygon is empty.
 *
 * The caller owns the returned buffer and must free() it when no longer needed.
 */
float *polygon_alloc_float_xy(const Polygon *poly, size_t *out_float_cnt);

#ifdef __cplusplus
}
#endif

#endif /* POLYGON_H */
