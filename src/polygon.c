#include "polygon.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Internal helper: ensure polygon has at least min_capacity vertices allocated. */
static bool polygon_ensure_capacity(Polygon *poly, size_t min_capacity) {
    if (!poly) {
        return false;
    }

    if (poly->capacity >= min_capacity) {
        return true;
    }

    size_t new_capacity = (poly->capacity == 0) ? 4U : poly->capacity;
    while (new_capacity < min_capacity) {
        /* Simple growth strategy: double until sufficient. */
        new_capacity *= 2U;
    }

    Point2D *new_vertices = (Point2D *)realloc(poly->vertices, new_capacity * sizeof(Point2D));
    if (!new_vertices) {
        return false;
    }

    poly->vertices = new_vertices;
    poly->capacity = new_capacity;
    return true;
}

void polygon_init(Polygon *poly) {
    if (!poly) {
        return;
    }
    poly->vertices = NULL;
    poly->count    = 0U;
    poly->capacity = 0U;
}

bool polygon_init_with_capacity(Polygon *poly, size_t capacity) {
    if (!poly) {
        return false;
    }

    polygon_init(poly);

    if (capacity == 0U) {
        return true;
    }

    poly->vertices = (Point2D *)malloc(capacity * sizeof(Point2D));
    if (!poly->vertices) {
        return false;
    }

    poly->capacity = capacity;
    return true;
}

void polygon_clear(Polygon *poly) {
    if (!poly) {
        return;
    }

    free(poly->vertices);
    poly->vertices = NULL;
    poly->count    = 0U;
    poly->capacity = 0U;
}

bool polygon_add_vertex(Polygon *poly, double x, double y) {
    if (!poly) {
        return false;
    }

    if (!polygon_ensure_capacity(poly, poly->count + 1U)) {
        return false;
    }

    poly->vertices[poly->count].x = x;
    poly->vertices[poly->count].y = y;
    poly->count += 1U;

    return true;
}

bool polygon_insert_vertex(Polygon *poly, size_t index, double x, double y) {
    if (!poly) {
        return false;
    }

    if (index > poly->count) {  /* index == count is allowed (append). */
        return false;
    }

    if (!polygon_ensure_capacity(poly, poly->count + 1U)) {
        return false;
    }

    if (index < poly->count) {
        /* Shift existing vertices to make room. */
        memmove(&poly->vertices[index + 1U],
                &poly->vertices[index],
                (poly->count - index) * sizeof(Point2D));
    }

    poly->vertices[index].x = x;
    poly->vertices[index].y = y;
    poly->count += 1U;

    return true;
}

bool polygon_remove_vertex(Polygon *poly, size_t index) {
    if (!poly) {
        return false;
    }

    if (index >= poly->count) {
        return false;
    }

    if (index + 1U < poly->count) {
        /* Shift remaining vertices left. */
        memmove(&poly->vertices[index],
                &poly->vertices[index + 1U],
                (poly->count - index - 1U) * sizeof(Point2D));
    }

    poly->count -= 1U;
    return true;
}

bool polygon_is_valid(const Polygon *poly) {
    return poly && (poly->count >= 3U);
}

void polygon_translate(Polygon *poly, double dx, double dy) {
    if (!poly) {
        return;
    }

    for (size_t i = 0U; i < poly->count; ++i) {
        poly->vertices[i].x += dx;
        poly->vertices[i].y += dy;
    }
}

void polygon_scale(Polygon *poly, double sx, double sy) {
    if (!poly) {
        return;
    }

    for (size_t i = 0U; i < poly->count; ++i) {
        poly->vertices[i].x *= sx;
        poly->vertices[i].y *= sy;
    }
}

void polygon_rotate(Polygon *poly, double radians) {
    if (!poly) {
        return;
    }

    const double c = cos(radians);
    const double s = sin(radians);

    for (size_t i = 0U; i < poly->count; ++i) {
        const double x = poly->vertices[i].x;
        const double y = poly->vertices[i].y;

        poly->vertices[i].x = c * x - s * y;
        poly->vertices[i].y = s * x + c * y;
    }
}

double polygon_perimeter(const Polygon *poly) {
    if (!poly || poly->count < 2U) {
        return 0.0;
    }

    double perimeter = 0.0;

    for (size_t i = 0U; i < poly->count; ++i) {
        const Point2D *a = &poly->vertices[i];
        const Point2D *b = &poly->vertices[(i + 1U) % poly->count];

        const double dx = b->x - a->x;
        const double dy = b->y - a->y;

        perimeter += hypot(dx, dy);
    }

    return perimeter;
}
