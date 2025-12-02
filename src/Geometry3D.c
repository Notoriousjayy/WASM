/**
 * geometry3d.c - 3D Geometry Primitives and Collision Detection (C23)
 * 
 * Translated from C++ to C23
 * Original: Geometry3D.cpp
 */

#include "geometry3D.h"

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Floating Point Comparison Macro
 ******************************************************************************/

#define CMP(x, y) \
    (fabsf((x) - (y)) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

/*******************************************************************************
 * Dynamic Array Implementations
 ******************************************************************************/

/* ContactArray */
void contact_array_init(ContactArray* arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void contact_array_free(ContactArray* arr) {
    if (arr->data) {
        free(arr->data);
        arr->data = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}

void contact_array_reserve(ContactArray* arr, int capacity) {
    if (capacity > arr->capacity) {
        arr->data = realloc(arr->data, (size_t)capacity * sizeof(vec3));
        arr->capacity = capacity;
    }
}

void contact_array_push(ContactArray* arr, vec3 point) {
    if (arr->count >= arr->capacity) {
        int new_cap = arr->capacity == 0 ? 8 : arr->capacity * 2;
        contact_array_reserve(arr, new_cap);
    }
    arr->data[arr->count++] = point;
}

void contact_array_clear(ContactArray* arr) {
    arr->count = 0;
}

void contact_array_erase(ContactArray* arr, int index) {
    if (index >= 0 && index < arr->count) {
        memmove(&arr->data[index], &arr->data[index + 1],
                (size_t)(arr->count - index - 1) * sizeof(vec3));
        arr->count--;
    }
}

/* Line3DArray */
void line3d_array_init(Line3DArray* arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void line3d_array_free(Line3DArray* arr) {
    if (arr->data) {
        free(arr->data);
        arr->data = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}

void line3d_array_reserve(Line3DArray* arr, int capacity) {
    if (capacity > arr->capacity) {
        arr->data = realloc(arr->data, (size_t)capacity * sizeof(Line3D));
        arr->capacity = capacity;
    }
}

void line3d_array_push(Line3DArray* arr, Line3D line) {
    if (arr->count >= arr->capacity) {
        int new_cap = arr->capacity == 0 ? 8 : arr->capacity * 2;
        line3d_array_reserve(arr, new_cap);
    }
    arr->data[arr->count++] = line;
}

/* PlaneArray */
void plane_array_init(PlaneArray* arr) {
    arr->data = NULL;
    arr->count = 0;
    arr->capacity = 0;
}

void plane_array_free(PlaneArray* arr) {
    if (arr->data) {
        free(arr->data);
        arr->data = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}

void plane_array_push(PlaneArray* arr, Plane plane) {
    if (arr->count >= arr->capacity) {
        int new_cap = arr->capacity == 0 ? 8 : arr->capacity * 2;
        arr->data = realloc(arr->data, (size_t)new_cap * sizeof(Plane));
        arr->capacity = new_cap;
    }
    arr->data[arr->count++] = plane;
}

/*******************************************************************************
 * RaycastResult / CollisionManifold
 ******************************************************************************/

void raycast_result_reset(RaycastResult* result) {
    if (result) {
        result->t = -1.0f;
        result->hit = false;
        result->normal = (vec3){0, 0, 1};
        result->point = (vec3){0, 0, 0};
    }
}

void collision_manifold_init(CollisionManifold* result) {
    if (result) {
        result->colliding = false;
        result->normal = (vec3){0, 0, 1};
        result->depth = FLT_MAX;
        contact_array_init(&result->contacts);
    }
}

void collision_manifold_free(CollisionManifold* result) {
    if (result) {
        contact_array_free(&result->contacts);
    }
}

void collision_manifold_reset(CollisionManifold* result) {
    if (result) {
        result->colliding = false;
        result->normal = (vec3){0, 0, 1};
        result->depth = FLT_MAX;
        contact_array_clear(&result->contacts);
    }
}

/*******************************************************************************
 * Line3D Operations
 ******************************************************************************/

float line3d_length(Line3D line) {
    return vec3_magnitude(vec3_sub(line.start, line.end));
}

float line3d_length_sq(Line3D line) {
    return vec3_magnitude_sq(vec3_sub(line.start, line.end));
}

/*******************************************************************************
 * Ray3D Operations
 ******************************************************************************/

Ray3D ray3d_from_points(Point3D from, Point3D to) {
    return ray3d_create(from, vec3_sub(to, from));
}

/*******************************************************************************
 * AABB Operations
 ******************************************************************************/

vec3 aabb_get_min(AABB aabb) {
    vec3 p1 = vec3_add(aabb.position, aabb.size);
    vec3 p2 = vec3_sub(aabb.position, aabb.size);
    return (vec3){
        fminf(p1.x, p2.x),
        fminf(p1.y, p2.y),
        fminf(p1.z, p2.z)
    };
}

vec3 aabb_get_max(AABB aabb) {
    vec3 p1 = vec3_add(aabb.position, aabb.size);
    vec3 p2 = vec3_sub(aabb.position, aabb.size);
    return (vec3){
        fmaxf(p1.x, p2.x),
        fmaxf(p1.y, p2.y),
        fmaxf(p1.z, p2.z)
    };
}

AABB aabb_from_min_max(vec3 min, vec3 max) {
    return aabb_create(
        vec3_scale(vec3_add(min, max), 0.5f),
        vec3_scale(vec3_sub(max, min), 0.5f)
    );
}

/*******************************************************************************
 * Plane Operations
 ******************************************************************************/

float plane_equation(Point3D point, Plane plane) {
    return vec3_dot(point, plane.normal) - plane.distance;
}

#ifndef NO_EXTRAS
float plane_equation_reversed(Plane plane, Point3D point) {
    return vec3_dot(point, plane.normal) - plane.distance;
}
#endif

Plane plane_from_triangle(Triangle t) {
    Plane result;
    result.normal = vec3_normalized(vec3_cross(
        vec3_sub(t.b, t.a),
        vec3_sub(t.c, t.a)
    ));
    result.distance = vec3_dot(result.normal, t.a);
    return result;
}

/*******************************************************************************
 * Debug Print Functions
 ******************************************************************************/

#ifndef NO_EXTRAS
void line3d_print(FILE* stream, Line3D shape) {
    fprintf(stream, "start: (%.4f, %.4f, %.4f), end: (%.4f, %.4f, %.4f)",
            shape.start.x, shape.start.y, shape.start.z,
            shape.end.x, shape.end.y, shape.end.z);
}

void ray3d_print(FILE* stream, Ray3D shape) {
    fprintf(stream, "origin: (%.4f, %.4f, %.4f), direction: (%.4f, %.4f, %.4f)",
            shape.origin.x, shape.origin.y, shape.origin.z,
            shape.direction.x, shape.direction.y, shape.direction.z);
}

void sphere_print(FILE* stream, Sphere shape) {
    fprintf(stream, "position: (%.4f, %.4f, %.4f), radius: %.4f",
            shape.position.x, shape.position.y, shape.position.z, shape.radius);
}

void aabb_print(FILE* stream, AABB shape) {
    vec3 min = aabb_get_min(shape);
    vec3 max = aabb_get_max(shape);
    fprintf(stream, "min: (%.4f, %.4f, %.4f), max: (%.4f, %.4f, %.4f)",
            min.x, min.y, min.z, max.x, max.y, max.z);
}

void plane_print(FILE* stream, Plane shape) {
    fprintf(stream, "normal: (%.4f, %.4f, %.4f), distance: %.4f",
            shape.normal.x, shape.normal.y, shape.normal.z, shape.distance);
}

void triangle_print(FILE* stream, Triangle shape) {
    fprintf(stream, "a: (%.4f, %.4f, %.4f), b: (%.4f, %.4f, %.4f), c: (%.4f, %.4f, %.4f)",
            shape.a.x, shape.a.y, shape.a.z,
            shape.b.x, shape.b.y, shape.b.z,
            shape.c.x, shape.c.y, shape.c.z);
}

void obb_print(FILE* stream, OBB shape) {
    fprintf(stream, "position: (%.4f, %.4f, %.4f), ", 
            shape.position.x, shape.position.y, shape.position.z);
    fprintf(stream, "size: (%.4f, %.4f, %.4f), ",
            shape.size.x, shape.size.y, shape.size.z);
    fprintf(stream, "x basis: (%.4f, %.4f, %.4f), ",
            shape.orientation.m[0][0], shape.orientation.m[1][0], shape.orientation.m[2][0]);
    fprintf(stream, "y basis: (%.4f, %.4f, %.4f), ",
            shape.orientation.m[0][1], shape.orientation.m[1][1], shape.orientation.m[2][1]);
    fprintf(stream, "z basis: (%.4f, %.4f, %.4f)",
            shape.orientation.m[0][2], shape.orientation.m[1][2], shape.orientation.m[2][2]);
}
#endif

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_in_sphere(Point3D point, Sphere sphere) {
    float dist_sq = vec3_magnitude_sq(vec3_sub(point, sphere.position));
    return dist_sq < sphere.radius * sphere.radius;
}

bool point_on_plane(Point3D point, Plane plane) {
    return CMP(vec3_dot(point, plane.normal) - plane.distance, 0.0f);
}

bool point_in_aabb(Point3D point, AABB aabb) {
    vec3 min = aabb_get_min(aabb);
    vec3 max = aabb_get_max(aabb);

    if (point.x < min.x || point.y < min.y || point.z < min.z) {
        return false;
    }
    if (point.x > max.x || point.y > max.y || point.z > max.z) {
        return false;
    }
    return true;
}

bool point_in_obb(Point3D point, OBB obb) {
    vec3 dir = vec3_sub(point, obb.position);

    for (int i = 0; i < 3; ++i) {
        vec3 axis = (vec3){
            obb.orientation.m[i][0],
            obb.orientation.m[i][1],
            obb.orientation.m[i][2]
        };
        float distance = vec3_dot(dir, axis);
        float extent = obb.size.v[i];

        if (distance > extent || distance < -extent) {
            return false;
        }
    }
    return true;
}

bool point_on_line3d(Point3D point, Line3D line) {
    Point3D closest = closest_point_on_line3d(line, point);
    float dist_sq = vec3_magnitude_sq(vec3_sub(closest, point));
    return CMP(dist_sq, 0.0f);
}

bool point_on_ray3d(Point3D point, Ray3D ray) {
    if (vec3_equal(point, ray.origin)) {
        return true;
    }
    vec3 norm = vec3_normalized(vec3_sub(point, ray.origin));
    float diff = vec3_dot(norm, ray.direction);
    return CMP(diff, 1.0f);
}

bool point_in_triangle(Point3D p, Triangle t) {
    vec3 a = vec3_sub(t.a, p);
    vec3 b = vec3_sub(t.b, p);
    vec3 c = vec3_sub(t.c, p);

    vec3 norm_pbc = vec3_cross(b, c);
    vec3 norm_pca = vec3_cross(c, a);
    vec3 norm_pab = vec3_cross(a, b);

    if (vec3_dot(norm_pbc, norm_pca) < 0.0f) {
        return false;
    }
    if (vec3_dot(norm_pbc, norm_pab) < 0.0f) {
        return false;
    }
    return true;
}

#ifndef NO_EXTRAS
bool point_in_plane(Point3D point, Plane plane) {
    return point_on_plane(point, plane);
}

bool point_in_line3d(Point3D point, Line3D line) {
    return point_on_line3d(point, line);
}

bool point_in_ray3d(Point3D point, Ray3D ray) {
    return point_on_ray3d(point, ray);
}
#endif

/*******************************************************************************
 * Closest Point Functions
 ******************************************************************************/

Point3D closest_point_on_sphere(Sphere sphere, Point3D point) {
    vec3 dir = vec3_sub(point, sphere.position);
    dir = vec3_normalized(dir);
    dir = vec3_scale(dir, sphere.radius);
    return vec3_add(dir, sphere.position);
}

Point3D closest_point_on_aabb(AABB aabb, Point3D point) {
    Point3D result = point;
    vec3 min = aabb_get_min(aabb);
    vec3 max = aabb_get_max(aabb);

    /* Clamp to min */
    result.x = (result.x < min.x) ? min.x : result.x;
    result.y = (result.y < min.y) ? min.y : result.y;
    result.z = (result.z < min.z) ? min.z : result.z;

    /* Clamp to max */
    result.x = (result.x > max.x) ? max.x : result.x;
    result.y = (result.y > max.y) ? max.y : result.y;
    result.z = (result.z > max.z) ? max.z : result.z;

    return result;
}

Point3D closest_point_on_obb(OBB obb, Point3D point) {
    Point3D result = obb.position;
    vec3 dir = vec3_sub(point, obb.position);

    for (int i = 0; i < 3; ++i) {
        vec3 axis = (vec3){
            obb.orientation.m[i][0],
            obb.orientation.m[i][1],
            obb.orientation.m[i][2]
        };
        float distance = vec3_dot(dir, axis);
        float extent = obb.size.v[i];

        if (distance > extent) {
            distance = extent;
        }
        if (distance < -extent) {
            distance = -extent;
        }
        result = vec3_add(result, vec3_scale(axis, distance));
    }
    return result;
}

Point3D closest_point_on_plane(Plane plane, Point3D point) {
    float distance = vec3_dot(plane.normal, point) - plane.distance;
    return vec3_sub(point, vec3_scale(plane.normal, distance));
}

Point3D closest_point_on_line3d(Line3D line, Point3D point) {
    vec3 line_vec = vec3_sub(line.end, line.start);
    float t = vec3_dot(vec3_sub(point, line.start), line_vec) / vec3_dot(line_vec, line_vec);
    t = fmaxf(t, 0.0f);
    t = fminf(t, 1.0f);
    return vec3_add(line.start, vec3_scale(line_vec, t));
}

Point3D closest_point_on_ray3d(Ray3D ray, Point3D point) {
    float t = vec3_dot(vec3_sub(point, ray.origin), ray.direction);
    t = fmaxf(t, 0.0f);
    return vec3_add(ray.origin, vec3_scale(ray.direction, t));
}

Point3D closest_point_on_triangle(Triangle t, Point3D p) {
    Plane plane = plane_from_triangle(t);
    Point3D closest = closest_point_on_plane(plane, p);

    if (point_in_triangle(closest, t)) {
        return closest;
    }

    Point3D c1 = closest_point_on_line3d(line3d_create(t.a, t.b), closest);
    Point3D c2 = closest_point_on_line3d(line3d_create(t.b, t.c), closest);
    Point3D c3 = closest_point_on_line3d(line3d_create(t.c, t.a), closest);

    float mag_sq1 = vec3_magnitude_sq(vec3_sub(closest, c1));
    float mag_sq2 = vec3_magnitude_sq(vec3_sub(closest, c2));
    float mag_sq3 = vec3_magnitude_sq(vec3_sub(closest, c3));

    if (mag_sq1 < mag_sq2 && mag_sq1 < mag_sq3) {
        return c1;
    }
    else if (mag_sq2 < mag_sq1 && mag_sq2 < mag_sq3) {
        return c2;
    }
    return c3;
}

/*******************************************************************************
 * Interval / SAT Functions
 ******************************************************************************/

Interval3D interval3d_from_triangle(Triangle triangle, vec3 axis) {
    Interval3D result;
    result.min = vec3_dot(axis, triangle.points[0]);
    result.max = result.min;

    for (int i = 1; i < 3; ++i) {
        float value = vec3_dot(axis, triangle.points[i]);
        result.min = fminf(result.min, value);
        result.max = fmaxf(result.max, value);
    }
    return result;
}

Interval3D interval3d_from_aabb(AABB aabb, vec3 axis) {
    vec3 mn = aabb_get_min(aabb);
    vec3 mx = aabb_get_max(aabb);

    vec3 vertices[8] = {
        {mn.x, mx.y, mx.z}, {mn.x, mx.y, mn.z},
        {mn.x, mn.y, mx.z}, {mn.x, mn.y, mn.z},
        {mx.x, mx.y, mx.z}, {mx.x, mx.y, mn.z},
        {mx.x, mn.y, mx.z}, {mx.x, mn.y, mn.z}
    };

    Interval3D result;
    result.min = result.max = vec3_dot(axis, vertices[0]);

    for (int i = 1; i < 8; ++i) {
        float proj = vec3_dot(axis, vertices[i]);
        result.min = fminf(result.min, proj);
        result.max = fmaxf(result.max, proj);
    }
    return result;
}

Interval3D interval3d_from_obb(OBB obb, vec3 axis) {
    vec3 C = obb.position;
    vec3 E = obb.size;
    vec3 A[3] = {
        {obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]},
        {obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]},
        {obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]}
    };

    vec3 vertices[8];
    vertices[0] = vec3_add(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[1] = vec3_add(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[2] = vec3_add(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[3] = vec3_sub(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[4] = vec3_sub(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[5] = vec3_sub(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[6] = vec3_sub(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    vertices[7] = vec3_add(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));

    Interval3D result;
    result.min = result.max = vec3_dot(axis, vertices[0]);

    for (int i = 1; i < 8; ++i) {
        float proj = vec3_dot(axis, vertices[i]);
        result.min = fminf(result.min, proj);
        result.max = fmaxf(result.max, proj);
    }
    return result;
}

bool overlap_on_axis_aabb_obb(AABB aabb, OBB obb, vec3 axis) {
    Interval3D a = interval3d_from_aabb(aabb, axis);
    Interval3D b = interval3d_from_obb(obb, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_obb_obb(OBB obb1, OBB obb2, vec3 axis) {
    Interval3D a = interval3d_from_obb(obb1, axis);
    Interval3D b = interval3d_from_obb(obb2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_aabb_triangle(AABB aabb, Triangle tri, vec3 axis) {
    Interval3D a = interval3d_from_aabb(aabb, axis);
    Interval3D b = interval3d_from_triangle(tri, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_obb_triangle(OBB obb, Triangle tri, vec3 axis) {
    Interval3D a = interval3d_from_obb(obb, axis);
    Interval3D b = interval3d_from_triangle(tri, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}

bool overlap_on_axis_triangle_triangle(Triangle t1, Triangle t2, vec3 axis) {
    Interval3D a = interval3d_from_triangle(t1, axis);
    Interval3D b = interval3d_from_triangle(t2, axis);
    return (b.min <= a.max) && (a.min <= b.max);
}
/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool sphere_sphere(Sphere s1, Sphere s2) {
    float radii_sum = s1.radius + s2.radius;
    float dist_sq = vec3_magnitude_sq(vec3_sub(s1.position, s2.position));
    return dist_sq < radii_sum * radii_sum;
}

bool sphere_aabb(Sphere sphere, AABB aabb) {
    Point3D closest = closest_point_on_aabb(aabb, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    float radius_sq = sphere.radius * sphere.radius;
    return dist_sq < radius_sq;
}

bool sphere_obb(Sphere sphere, OBB obb) {
    Point3D closest = closest_point_on_obb(obb, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    float radius_sq = sphere.radius * sphere.radius;
    return dist_sq < radius_sq;
}

bool sphere_plane(Sphere sphere, Plane plane) {
    Point3D closest = closest_point_on_plane(plane, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    float radius_sq = sphere.radius * sphere.radius;
    return dist_sq < radius_sq;
}

bool aabb_aabb(AABB a1, AABB a2) {
    vec3 a_min = aabb_get_min(a1);
    vec3 a_max = aabb_get_max(a1);
    vec3 b_min = aabb_get_min(a2);
    vec3 b_max = aabb_get_max(a2);

    return (a_min.x <= b_max.x && a_max.x >= b_min.x) &&
           (a_min.y <= b_max.y && a_max.y >= b_min.y) &&
           (a_min.z <= b_max.z && a_max.z >= b_min.z);
}

bool aabb_obb(AABB aabb, OBB obb) {
    vec3 test[15] = {
        {1, 0, 0},  /* AABB axis 1 */
        {0, 1, 0},  /* AABB axis 2 */
        {0, 0, 1},  /* AABB axis 3 */
        {obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]},
        {obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]},
        {obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]}
    };

    for (int i = 0; i < 3; ++i) {
        test[6 + i * 3 + 0] = vec3_cross(test[i], test[3]);
        test[6 + i * 3 + 1] = vec3_cross(test[i], test[4]);
        test[6 + i * 3 + 2] = vec3_cross(test[i], test[5]);
    }

    for (int i = 0; i < 15; ++i) {
        if (!overlap_on_axis_aabb_obb(aabb, obb, test[i])) {
            return false;
        }
    }
    return true;
}

bool aabb_plane(AABB aabb, Plane plane) {
    float p_len = aabb.size.x * fabsf(plane.normal.x) +
                  aabb.size.y * fabsf(plane.normal.y) +
                  aabb.size.z * fabsf(plane.normal.z);
    float dist = vec3_dot(plane.normal, aabb.position) - plane.distance;
    return fabsf(dist) <= p_len;
}

bool obb_obb(OBB o1, OBB o2) {
    vec3 test[15] = {
        {o1.orientation.m[0][0], o1.orientation.m[0][1], o1.orientation.m[0][2]},
        {o1.orientation.m[1][0], o1.orientation.m[1][1], o1.orientation.m[1][2]},
        {o1.orientation.m[2][0], o1.orientation.m[2][1], o1.orientation.m[2][2]},
        {o2.orientation.m[0][0], o2.orientation.m[0][1], o2.orientation.m[0][2]},
        {o2.orientation.m[1][0], o2.orientation.m[1][1], o2.orientation.m[1][2]},
        {o2.orientation.m[2][0], o2.orientation.m[2][1], o2.orientation.m[2][2]}
    };

    for (int i = 0; i < 3; ++i) {
        test[6 + i * 3 + 0] = vec3_cross(test[i], test[3]);
        test[6 + i * 3 + 1] = vec3_cross(test[i], test[4]);
        test[6 + i * 3 + 2] = vec3_cross(test[i], test[5]);
    }

    for (int i = 0; i < 15; ++i) {
        if (!overlap_on_axis_obb_obb(o1, o2, test[i])) {
            return false;
        }
    }
    return true;
}

bool obb_plane(OBB obb, Plane plane) {
    vec3 rot[3] = {
        {obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]},
        {obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]},
        {obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]}
    };

    float p_len = obb.size.x * fabsf(vec3_dot(plane.normal, rot[0])) +
                  obb.size.y * fabsf(vec3_dot(plane.normal, rot[1])) +
                  obb.size.z * fabsf(vec3_dot(plane.normal, rot[2]));
    float dist = vec3_dot(plane.normal, obb.position) - plane.distance;
    return fabsf(dist) <= p_len;
}

bool plane_plane(Plane p1, Plane p2) {
    vec3 d = vec3_cross(p1.normal, p2.normal);
    return !CMP(vec3_dot(d, d), 0.0f);
}

bool triangle_sphere(Triangle t, Sphere s) {
    Point3D closest = closest_point_on_triangle(t, s.position);
    float mag_sq = vec3_magnitude_sq(vec3_sub(closest, s.position));
    return mag_sq <= s.radius * s.radius;
}

bool triangle_aabb(Triangle t, AABB a) {
    vec3 f0 = vec3_sub(t.b, t.a);
    vec3 f1 = vec3_sub(t.c, t.b);
    vec3 f2 = vec3_sub(t.a, t.c);

    vec3 u0 = {1.0f, 0.0f, 0.0f};
    vec3 u1 = {0.0f, 1.0f, 0.0f};
    vec3 u2 = {0.0f, 0.0f, 1.0f};

    vec3 test[13] = {
        u0, u1, u2,
        vec3_cross(f0, f1),
        vec3_cross(u0, f0), vec3_cross(u0, f1), vec3_cross(u0, f2),
        vec3_cross(u1, f0), vec3_cross(u1, f1), vec3_cross(u1, f2),
        vec3_cross(u2, f0), vec3_cross(u2, f1), vec3_cross(u2, f2)
    };

    for (int i = 0; i < 13; ++i) {
        if (!overlap_on_axis_aabb_triangle(a, t, test[i])) {
            return false;
        }
    }
    return true;
}

bool triangle_obb(Triangle t, OBB o) {
    vec3 f0 = vec3_sub(t.b, t.a);
    vec3 f1 = vec3_sub(t.c, t.b);
    vec3 f2 = vec3_sub(t.a, t.c);

    vec3 u0 = {o.orientation.m[0][0], o.orientation.m[0][1], o.orientation.m[0][2]};
    vec3 u1 = {o.orientation.m[1][0], o.orientation.m[1][1], o.orientation.m[1][2]};
    vec3 u2 = {o.orientation.m[2][0], o.orientation.m[2][1], o.orientation.m[2][2]};

    vec3 test[13] = {
        u0, u1, u2,
        vec3_cross(f0, f1),
        vec3_cross(u0, f0), vec3_cross(u0, f1), vec3_cross(u0, f2),
        vec3_cross(u1, f0), vec3_cross(u1, f1), vec3_cross(u1, f2),
        vec3_cross(u2, f0), vec3_cross(u2, f1), vec3_cross(u2, f2)
    };

    for (int i = 0; i < 13; ++i) {
        if (!overlap_on_axis_obb_triangle(o, t, test[i])) {
            return false;
        }
    }
    return true;
}

bool triangle_plane(Triangle t, Plane p) {
    float side1 = plane_equation(t.a, p);
    float side2 = plane_equation(t.b, p);
    float side3 = plane_equation(t.c, p);

    if (CMP(side1, 0.0f) && CMP(side2, 0.0f) && CMP(side3, 0.0f)) {
        return true;
    }
    if (side1 > 0 && side2 > 0 && side3 > 0) {
        return false;
    }
    if (side1 < 0 && side2 < 0 && side3 < 0) {
        return false;
    }
    return true;
}

vec3 sat_cross_edge(vec3 a, vec3 b, vec3 c, vec3 d) {
    vec3 ab = vec3_sub(b, a);
    vec3 cd = vec3_sub(d, c);

    vec3 result = vec3_cross(ab, cd);
    if (!CMP(vec3_magnitude_sq(result), 0.0f)) {
        return result;
    }

    vec3 axis = vec3_cross(ab, vec3_sub(c, a));
    result = vec3_cross(ab, axis);
    if (!CMP(vec3_magnitude_sq(result), 0.0f)) {
        return result;
    }

    return (vec3){0, 0, 0};
}

bool triangle_triangle(Triangle t1, Triangle t2) {
    vec3 t1_f0 = vec3_sub(t1.b, t1.a);
    vec3 t1_f1 = vec3_sub(t1.c, t1.b);
    vec3 t1_f2 = vec3_sub(t1.a, t1.c);

    vec3 t2_f0 = vec3_sub(t2.b, t2.a);
    vec3 t2_f1 = vec3_sub(t2.c, t2.b);
    vec3 t2_f2 = vec3_sub(t2.a, t2.c);

    vec3 axes[11] = {
        vec3_cross(t1_f0, t1_f1),
        vec3_cross(t2_f0, t2_f1),
        vec3_cross(t2_f0, t1_f0), vec3_cross(t2_f0, t1_f1), vec3_cross(t2_f0, t1_f2),
        vec3_cross(t2_f1, t1_f0), vec3_cross(t2_f1, t1_f1), vec3_cross(t2_f1, t1_f2),
        vec3_cross(t2_f2, t1_f0), vec3_cross(t2_f2, t1_f1), vec3_cross(t2_f2, t1_f2)
    };

    for (int i = 0; i < 11; ++i) {
        if (!overlap_on_axis_triangle_triangle(t1, t2, axes[i])) {
            return false;
        }
    }
    return true;
}

bool triangle_triangle_robust(Triangle t1, Triangle t2) {
    vec3 axes[11] = {
        sat_cross_edge(t1.a, t1.b, t1.b, t1.c),
        sat_cross_edge(t2.a, t2.b, t2.b, t2.c),
        sat_cross_edge(t2.a, t2.b, t1.a, t1.b),
        sat_cross_edge(t2.a, t2.b, t1.b, t1.c),
        sat_cross_edge(t2.a, t2.b, t1.c, t1.a),
        sat_cross_edge(t2.b, t2.c, t1.a, t1.b),
        sat_cross_edge(t2.b, t2.c, t1.b, t1.c),
        sat_cross_edge(t2.b, t2.c, t1.c, t1.a),
        sat_cross_edge(t2.c, t2.a, t1.a, t1.b),
        sat_cross_edge(t2.c, t2.a, t1.b, t1.c),
        sat_cross_edge(t2.c, t2.a, t1.c, t1.a)
    };

    for (int i = 0; i < 11; ++i) {
        if (!overlap_on_axis_triangle_triangle(t1, t2, axes[i])) {
            if (!CMP(vec3_magnitude_sq(axes[i]), 0.0f)) {
                return false;
            }
        }
    }
    return true;
}

/*******************************************************************************
 * Raycasting
 ******************************************************************************/

bool raycast_sphere(Sphere sphere, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    vec3 e = vec3_sub(sphere.position, ray.origin);
    float r_sq = sphere.radius * sphere.radius;
    float e_sq = vec3_magnitude_sq(e);
    float a = vec3_dot(e, ray.direction);
    float b_sq = e_sq - (a * a);
    float f = sqrtf(fabsf(r_sq - b_sq));

    float t = a - f;

    if (r_sq - (e_sq - a * a) < 0.0f) {
        return false;
    }
    if (e_sq < r_sq) {
        t = a + f;
    }

    if (out_result) {
        out_result->t = t;
        out_result->hit = true;
        out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t));
        out_result->normal = vec3_normalized(vec3_sub(out_result->point, sphere.position));
    }
    return true;
}

bool raycast_aabb(AABB aabb, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    vec3 min = aabb_get_min(aabb);
    vec3 max = aabb_get_max(aabb);

    float t1 = (min.x - ray.origin.x) / (CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
    float t2 = (max.x - ray.origin.x) / (CMP(ray.direction.x, 0.0f) ? 0.00001f : ray.direction.x);
    float t3 = (min.y - ray.origin.y) / (CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
    float t4 = (max.y - ray.origin.y) / (CMP(ray.direction.y, 0.0f) ? 0.00001f : ray.direction.y);
    float t5 = (min.z - ray.origin.z) / (CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);
    float t6 = (max.z - ray.origin.z) / (CMP(ray.direction.z, 0.0f) ? 0.00001f : ray.direction.z);

    float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
    float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

    if (tmax < 0 || tmin > tmax) {
        return false;
    }

    float t_result = (tmin < 0.0f) ? tmax : tmin;

    if (out_result) {
        out_result->t = t_result;
        out_result->hit = true;
        out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t_result));

        vec3 normals[6] = {{-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1}};
        float t_vals[6] = {t1, t2, t3, t4, t5, t6};
        for (int i = 0; i < 6; ++i) {
            if (CMP(t_result, t_vals[i])) {
                out_result->normal = normals[i];
            }
        }
    }
    return true;
}

bool raycast_obb(OBB obb, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    vec3 p = vec3_sub(obb.position, ray.origin);

    vec3 X = {obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]};
    vec3 Y = {obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]};
    vec3 Z = {obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]};

    vec3 f = {vec3_dot(X, ray.direction), vec3_dot(Y, ray.direction), vec3_dot(Z, ray.direction)};
    vec3 e = {vec3_dot(X, p), vec3_dot(Y, p), vec3_dot(Z, p)};

    float t[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        if (CMP(f.v[i], 0.0f)) {
            if (-e.v[i] - obb.size.v[i] > 0 || -e.v[i] + obb.size.v[i] < 0) {
                return false;
            }
            f.v[i] = 0.00001f;
        }
        t[i * 2 + 0] = (e.v[i] + obb.size.v[i]) / f.v[i];
        t[i * 2 + 1] = (e.v[i] - obb.size.v[i]) / f.v[i];
    }

    float tmin = fmaxf(fmaxf(fminf(t[0], t[1]), fminf(t[2], t[3])), fminf(t[4], t[5]));
    float tmax = fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));

    if (tmax < 0 || tmin > tmax) {
        return false;
    }

    float t_result = (tmin < 0.0f) ? tmax : tmin;

    if (out_result) {
        out_result->hit = true;
        out_result->t = t_result;
        out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t_result));

        vec3 normals[6] = {X, vec3_scale(X, -1.0f), Y, vec3_scale(Y, -1.0f), Z, vec3_scale(Z, -1.0f)};
        for (int i = 0; i < 6; ++i) {
            if (CMP(t_result, t[i])) {
                out_result->normal = vec3_normalized(normals[i]);
            }
        }
    }
    return true;
}

bool raycast_plane(Plane plane, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    float nd = vec3_dot(ray.direction, plane.normal);
    float pn = vec3_dot(ray.origin, plane.normal);

    if (nd >= 0.0f) {
        return false;
    }

    float t = (plane.distance - pn) / nd;

    if (t >= 0.0f) {
        if (out_result) {
            out_result->t = t;
            out_result->hit = true;
            out_result->point = vec3_add(ray.origin, vec3_scale(ray.direction, t));
            out_result->normal = vec3_normalized(plane.normal);
        }
        return true;
    }
    return false;
}

/*******************************************************************************
 * Barycentric Coordinates
 ******************************************************************************/

#ifndef NO_EXTRAS
vec3 barycentric_optimized(Point3D p, Triangle t) {
    vec3 v0 = vec3_sub(t.b, t.a);
    vec3 v1 = vec3_sub(t.c, t.a);
    vec3 v2 = vec3_sub(p, t.a);

    float d00 = vec3_dot(v0, v0);
    float d01 = vec3_dot(v0, v1);
    float d11 = vec3_dot(v1, v1);
    float d20 = vec3_dot(v2, v0);
    float d21 = vec3_dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;

    if (CMP(denom, 0.0f)) {
        return (vec3){0, 0, 0};
    }

    vec3 result;
    result.y = (d11 * d20 - d01 * d21) / denom;
    result.z = (d00 * d21 - d01 * d20) / denom;
    result.x = 1.0f - result.y - result.z;
    return result;
}

vec3 triangle_centroid(Triangle t) {
    vec3 result;
    result.x = t.a.x + t.b.x + t.c.x;
    result.y = t.a.y + t.b.y + t.c.y;
    result.z = t.a.z + t.b.z + t.c.z;
    return vec3_scale(result, 1.0f / 3.0f);
}
#endif

vec3 barycentric(Point3D p, Triangle t) {
    vec3 ap = vec3_sub(p, t.a);
    vec3 bp = vec3_sub(p, t.b);
    vec3 cp = vec3_sub(p, t.c);

    vec3 ab = vec3_sub(t.b, t.a);
    vec3 ac = vec3_sub(t.c, t.a);
    vec3 bc = vec3_sub(t.c, t.b);
    vec3 cb = vec3_sub(t.b, t.c);
    vec3 ca = vec3_sub(t.a, t.c);

    vec3 v = vec3_sub(ab, vec3_project(ab, cb));
    float a_coord = 1.0f - (vec3_dot(v, ap) / vec3_dot(v, ab));

    v = vec3_sub(bc, vec3_project(bc, ac));
    float b_coord = 1.0f - (vec3_dot(v, bp) / vec3_dot(v, bc));

    v = vec3_sub(ca, vec3_project(ca, ab));
    float c_coord = 1.0f - (vec3_dot(v, cp) / vec3_dot(v, ca));

    return (vec3){a_coord, b_coord, c_coord};
}

bool raycast_triangle(Triangle triangle, Ray3D ray, RaycastResult* out_result) {
    raycast_result_reset(out_result);

    Plane plane = plane_from_triangle(triangle);
    RaycastResult plane_result;
    if (!raycast_plane(plane, ray, &plane_result)) {
        return false;
    }
    float t = plane_result.t;

    Point3D result_point = vec3_add(ray.origin, vec3_scale(ray.direction, t));
    vec3 bary = barycentric(result_point, triangle);

    if (bary.x >= 0.0f && bary.x <= 1.0f &&
        bary.y >= 0.0f && bary.y <= 1.0f &&
        bary.z >= 0.0f && bary.z <= 1.0f) {

        if (out_result) {
            out_result->t = t;
            out_result->hit = true;
            out_result->point = result_point;
            out_result->normal = plane.normal;
        }
        return true;
    }
    return false;
}

/*******************************************************************************
 * Line Tests
 ******************************************************************************/

bool linetest_sphere(Sphere sphere, Line3D line) {
    Point3D closest = closest_point_on_line3d(line, sphere.position);
    float dist_sq = vec3_magnitude_sq(vec3_sub(sphere.position, closest));
    return dist_sq <= (sphere.radius * sphere.radius);
}

bool linetest_plane(Plane plane, Line3D line) {
    vec3 ab = vec3_sub(line.end, line.start);
    float n_a = vec3_dot(plane.normal, line.start);
    float n_ab = vec3_dot(plane.normal, ab);

    if (CMP(n_ab, 0.0f)) {
        return false;
    }

    float t = (plane.distance - n_a) / n_ab;
    return t >= 0.0f && t <= 1.0f;
}

bool linetest_aabb(AABB aabb, Line3D line) {
    Ray3D ray = ray3d_create(line.start, vec3_sub(line.end, line.start));
    RaycastResult raycast;
    if (!raycast_aabb(aabb, ray, &raycast)) {
        return false;
    }
    float t = raycast.t;
    return t >= 0 && t * t <= line3d_length_sq(line);
}

bool linetest_obb(OBB obb, Line3D line) {
    if (vec3_magnitude_sq(vec3_sub(line.end, line.start)) < 0.0000001f) {
        return point_in_obb(line.start, obb);
    }
    Ray3D ray = ray3d_create(line.start, vec3_sub(line.end, line.start));
    RaycastResult result;
    if (!raycast_obb(obb, ray, &result)) {
        return false;
    }
    float t = result.t;
    return t >= 0 && t * t <= line3d_length_sq(line);
}

bool linetest_triangle(Triangle triangle, Line3D line) {
    Ray3D ray = ray3d_create(line.start, vec3_sub(line.end, line.start));
    RaycastResult raycast;
    if (!raycast_triangle(triangle, ray, &raycast)) {
        return false;
    }
    float t = raycast.t;
    return t >= 0 && t * t <= line3d_length_sq(line);
}
/*******************************************************************************
 * BVH Stack (replaces std::list<BVHNode*>)
 ******************************************************************************/

typedef struct BVHStack {
    BVHNode** nodes;
    int       count;
    int       capacity;
} BVHStack;

static void bvh_stack_init(BVHStack* s, int cap) {
    s->nodes = malloc((size_t)cap * sizeof(BVHNode*));
    s->count = 0;
    s->capacity = cap;
}

static void bvh_stack_free(BVHStack* s) {
    free(s->nodes);
    s->nodes = NULL;
    s->count = 0;
    s->capacity = 0;
}

static void bvh_stack_push(BVHStack* s, BVHNode* node) {
    if (s->count >= s->capacity) {
        s->capacity *= 2;
        s->nodes = realloc(s->nodes, (size_t)s->capacity * sizeof(BVHNode*));
    }
    s->nodes[s->count++] = node;
}

static BVHNode* bvh_stack_pop(BVHStack* s) {
    return s->count > 0 ? s->nodes[--s->count] : NULL;
}

static bool bvh_stack_empty(BVHStack* s) {
    return s->count == 0;
}

/*******************************************************************************
 * BVH / Mesh Operations
 ******************************************************************************/

void mesh_accelerate(Mesh* mesh) {
    if (mesh->accelerator != NULL) {
        return;
    }

    vec3 min = mesh->vertices[0];
    vec3 max = mesh->vertices[0];

    for (int i = 1; i < mesh->num_triangles * 3; ++i) {
        min.x = fminf(mesh->vertices[i].x, min.x);
        min.y = fminf(mesh->vertices[i].y, min.y);
        min.z = fminf(mesh->vertices[i].z, min.z);

        max.x = fmaxf(mesh->vertices[i].x, max.x);
        max.y = fmaxf(mesh->vertices[i].y, max.y);
        max.z = fmaxf(mesh->vertices[i].z, max.z);
    }

    mesh->accelerator = malloc(sizeof(BVHNode));
    *mesh->accelerator = bvhnode_default();
    mesh->accelerator->bounds = aabb_from_min_max(min, max);
    mesh->accelerator->children = NULL;
    mesh->accelerator->num_triangles = mesh->num_triangles;
    mesh->accelerator->triangles = malloc((size_t)mesh->num_triangles * sizeof(int));

    for (int i = 0; i < mesh->num_triangles; ++i) {
        mesh->accelerator->triangles[i] = i;
    }

    bvhnode_split(mesh->accelerator, mesh, 3);
}

void bvhnode_split(BVHNode* node, const Mesh* mesh, int depth) {
    if (depth-- <= 0) {
        return;
    }

    if (node->children == NULL) {
        if (node->num_triangles > 0) {
            node->children = malloc(8 * sizeof(BVHNode));

            vec3 c = node->bounds.position;
            vec3 e = vec3_scale(node->bounds.size, 0.5f);

            vec3 offsets[8] = {
                {-e.x, +e.y, -e.z}, {+e.x, +e.y, -e.z},
                {-e.x, +e.y, +e.z}, {+e.x, +e.y, +e.z},
                {-e.x, -e.y, -e.z}, {+e.x, -e.y, -e.z},
                {-e.x, -e.y, +e.z}, {+e.x, -e.y, +e.z}
            };

            for (int i = 0; i < 8; ++i) {
                node->children[i] = bvhnode_default();
                node->children[i].bounds = aabb_create(vec3_add(c, offsets[i]), e);
            }
        }
    }

    if (node->children != NULL && node->num_triangles > 0) {
        for (int i = 0; i < 8; ++i) {
            node->children[i].num_triangles = 0;
            for (int j = 0; j < node->num_triangles; ++j) {
                Triangle t = mesh->triangles[node->triangles[j]];
                if (triangle_aabb(t, node->children[i].bounds)) {
                    node->children[i].num_triangles += 1;
                }
            }

            if (node->children[i].num_triangles == 0) {
                continue;
            }

            node->children[i].triangles = malloc((size_t)node->children[i].num_triangles * sizeof(int));
            int index = 0;
            for (int j = 0; j < node->num_triangles; ++j) {
                Triangle t = mesh->triangles[node->triangles[j]];
                if (triangle_aabb(t, node->children[i].bounds)) {
                    node->children[i].triangles[index++] = node->triangles[j];
                }
            }
        }

        node->num_triangles = 0;
        free(node->triangles);
        node->triangles = NULL;

        for (int i = 0; i < 8; ++i) {
            bvhnode_split(&node->children[i], mesh, depth);
        }
    }
}

void bvhnode_free(BVHNode* node) {
    if (node->children != NULL) {
        for (int i = 0; i < 8; ++i) {
            bvhnode_free(&node->children[i]);
        }
        free(node->children);
        node->children = NULL;
    }

    if (node->num_triangles != 0 || node->triangles != NULL) {
        free(node->triangles);
        node->triangles = NULL;
        node->num_triangles = 0;
    }
}

bool mesh_aabb(const Mesh* mesh, AABB aabb) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_aabb(mesh->triangles[i], aabb)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_aabb(mesh->triangles[node->triangles[i]], aabb)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (aabb_aabb(node->children[i].bounds, aabb)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool linetest_mesh(const Mesh* mesh, Line3D line) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (linetest_triangle(mesh->triangles[i], line)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (linetest_triangle(mesh->triangles[node->triangles[i]], line)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (linetest_aabb(node->children[i].bounds, line)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_sphere(const Mesh* mesh, Sphere sphere) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_sphere(mesh->triangles[i], sphere)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_sphere(mesh->triangles[node->triangles[i]], sphere)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (sphere_aabb(sphere, node->children[i].bounds)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_obb(const Mesh* mesh, OBB obb) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_obb(mesh->triangles[i], obb)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_obb(mesh->triangles[node->triangles[i]], obb)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (aabb_obb(node->children[i].bounds, obb)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_plane(const Mesh* mesh, Plane plane) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_plane(mesh->triangles[i], plane)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_plane(mesh->triangles[node->triangles[i]], plane)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (aabb_plane(node->children[i].bounds, plane)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

bool mesh_triangle(const Mesh* mesh, Triangle triangle) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            if (triangle_triangle(mesh->triangles[i], triangle)) {
                return true;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    if (triangle_triangle(mesh->triangles[node->triangles[i]], triangle)) {
                        bvh_stack_free(&stack);
                        return true;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    if (triangle_aabb(triangle, node->children[i].bounds)) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return false;
}

float mesh_ray(const Mesh* mesh, Ray3D ray) {
    if (mesh->accelerator == NULL) {
        for (int i = 0; i < mesh->num_triangles; ++i) {
            RaycastResult raycast;
            raycast_triangle(mesh->triangles[i], ray, &raycast);
            float result = raycast.t;
            if (result >= 0) {
                return result;
            }
        }
    }
    else {
        BVHStack stack;
        bvh_stack_init(&stack, 64);
        bvh_stack_push(&stack, mesh->accelerator);

        while (!bvh_stack_empty(&stack)) {
            BVHNode* node = bvh_stack_pop(&stack);

            if (node->num_triangles >= 0) {
                for (int i = 0; i < node->num_triangles; ++i) {
                    RaycastResult raycast;
                    raycast_triangle(mesh->triangles[node->triangles[i]], ray, &raycast);
                    float r = raycast.t;
                    if (r >= 0) {
                        bvh_stack_free(&stack);
                        return r;
                    }
                }
            }

            if (node->children != NULL) {
                for (int i = 7; i >= 0; --i) {
                    RaycastResult raycast;
                    raycast_aabb(node->children[i].bounds, ray, &raycast);
                    if (raycast.t >= 0) {
                        bvh_stack_push(&stack, &node->children[i]);
                    }
                }
            }
        }
        bvh_stack_free(&stack);
    }
    return -1.0f;
}

#ifndef NO_EXTRAS
float raycast_mesh(const Mesh* mesh, Ray3D ray) {
    return mesh_ray(mesh, ray);
}

float raycast_model(const Model* model, Ray3D ray) {
    return model_ray(model, ray);
}
#endif

/*******************************************************************************
 * Model Operations
 ******************************************************************************/

void model_set_content(Model* model, Mesh* mesh) {
    model->content = mesh;
    if (mesh != NULL) {
        vec3 min = mesh->vertices[0];
        vec3 max = mesh->vertices[0];

        for (int i = 1; i < mesh->num_triangles * 3; ++i) {
            min.x = fminf(mesh->vertices[i].x, min.x);
            min.y = fminf(mesh->vertices[i].y, min.y);
            min.z = fminf(mesh->vertices[i].z, min.z);

            max.x = fmaxf(mesh->vertices[i].x, max.x);
            max.y = fmaxf(mesh->vertices[i].y, max.y);
            max.z = fmaxf(mesh->vertices[i].z, max.z);
        }
        model->bounds = aabb_from_min_max(min, max);
    }
}

Mesh* model_get_mesh(const Model* model) {
    return model->content;
}

AABB model_get_bounds(const Model* model) {
    return model->bounds;
}

mat4 model_get_world_matrix(const Model* model) {
    mat4 translation = mat4_translate_vec3(model->position);
    mat4 rotation = Rotation(model->rotation.x, model->rotation.y, model->rotation.z);
    mat4 local_mat = mat4_mul(rotation, translation);

    mat4 parent_mat = mat4_identity();
    if (model->parent != NULL) {
        parent_mat = model_get_world_matrix(model->parent);
    }

    return mat4_mul(local_mat, parent_mat);
}

OBB model_get_obb(const Model* model) {
    mat4 world = model_get_world_matrix(model);
    AABB aabb = model->bounds;
    OBB obb;

    obb.size = aabb.size;
    obb.position = MultiplyPoint(aabb.position, world);
    obb.orientation = mat4_cut(world, 3, 3);

    return obb;
}

float model_ray(const Model* model, Ray3D ray) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Ray3D local;
    local.origin = MultiplyPoint(ray.origin, inv);
    local.direction = mat4_multiply_vector(ray.direction, inv);
    ray3d_normalize_direction(&local);

    if (model->content != NULL) {
        return mesh_ray(model->content, local);
    }
    return -1.0f;
}

bool linetest_model(const Model* model, Line3D line) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Line3D local;
    local.start = MultiplyPoint(line.start, inv);
    local.end = MultiplyPoint(line.end, inv);

    if (model->content != NULL) {
        return linetest_mesh(model->content, local);
    }
    return false;
}

bool model_sphere(const Model* model, Sphere sphere) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Sphere local;
    local.position = MultiplyPoint(sphere.position, inv);
    local.radius = sphere.radius;

    if (model->content != NULL) {
        return mesh_sphere(model->content, local);
    }
    return false;
}

bool model_aabb(const Model* model, AABB aabb) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    OBB local;
    local.size = aabb.size;
    local.position = MultiplyPoint(aabb.position, inv);
    local.orientation = mat4_cut(inv, 3, 3);

    if (model->content != NULL) {
        return mesh_obb(model->content, local);
    }
    return false;
}

bool model_obb(const Model* model, OBB obb) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    OBB local;
    local.size = obb.size;
    local.position = MultiplyPoint(obb.position, inv);
    local.orientation = mat3_mul(obb.orientation, mat4_cut(inv, 3, 3));

    if (model->content != NULL) {
        return mesh_obb(model->content, local);
    }
    return false;
}

bool model_plane(const Model* model, Plane plane) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Plane local;
    local.normal = mat4_multiply_vector(plane.normal, inv);
    local.distance = plane.distance;

    if (model->content != NULL) {
        return mesh_plane(model->content, local);
    }
    return false;
}

bool model_triangle(const Model* model, Triangle triangle) {
    mat4 world = model_get_world_matrix(model);
    mat4 inv = mat4_inverse(world);

    Triangle local;
    local.a = MultiplyPoint(triangle.a, inv);
    local.b = MultiplyPoint(triangle.b, inv);
    local.c = MultiplyPoint(triangle.c, inv);

    if (model->content != NULL) {
        return mesh_triangle(model->content, local);
    }
    return false;
}
/*******************************************************************************
 * Frustum Operations
 ******************************************************************************/

Point3D plane_intersection(Plane p1, Plane p2, Plane p3) {
    mat3 D = {{
        {p1.normal.x, p2.normal.x, p3.normal.x},
        {p1.normal.y, p2.normal.y, p3.normal.y},
        {p1.normal.z, p2.normal.z, p3.normal.z}
    }};

    vec3 A = {-p1.distance, -p2.distance, -p3.distance};

    mat3 Dx = D, Dy = D, Dz = D;
    Dx.m[0][0] = A.x; Dx.m[0][1] = A.y; Dx.m[0][2] = A.z;
    Dy.m[1][0] = A.x; Dy.m[1][1] = A.y; Dy.m[1][2] = A.z;
    Dz.m[2][0] = A.x; Dz.m[2][1] = A.y; Dz.m[2][2] = A.z;

    float det_d = mat3_determinant(D);

    if (CMP(det_d, 0.0f)) {
        return (Point3D){0, 0, 0};
    }

    float det_dx = mat3_determinant(Dx);
    float det_dy = mat3_determinant(Dy);
    float det_dz = mat3_determinant(Dz);

    return (Point3D){det_dx / det_d, det_dy / det_d, det_dz / det_d};
}

void frustum_get_corners(Frustum f, vec3* out_corners) {
    out_corners[0] = plane_intersection(f.near_plane, f.top, f.left);
    out_corners[1] = plane_intersection(f.near_plane, f.top, f.right);
    out_corners[2] = plane_intersection(f.near_plane, f.bottom, f.left);
    out_corners[3] = plane_intersection(f.near_plane, f.bottom, f.right);
    out_corners[4] = plane_intersection(f.far_plane, f.top, f.left);
    out_corners[5] = plane_intersection(f.far_plane, f.top, f.right);
    out_corners[6] = plane_intersection(f.far_plane, f.bottom, f.left);
    out_corners[7] = plane_intersection(f.far_plane, f.bottom, f.right);
}

float classify_aabb(AABB aabb, Plane plane) {
    float r = fabsf(aabb.size.x * plane.normal.x) +
              fabsf(aabb.size.y * plane.normal.y) +
              fabsf(aabb.size.z * plane.normal.z);

    float d = vec3_dot(plane.normal, aabb.position) + plane.distance;

    if (fabsf(d) < r) {
        return 0.0f;
    }
    else if (d < 0.0f) {
        return d + r;
    }
    return d - r;
}

float classify_obb(OBB obb, Plane plane) {
    vec3 normal = mat3_multiply_vector(plane.normal, obb.orientation);

    float r = fabsf(obb.size.x * normal.x) +
              fabsf(obb.size.y * normal.y) +
              fabsf(obb.size.z * normal.z);

    float d = vec3_dot(plane.normal, obb.position) + plane.distance;

    if (fabsf(d) < r) {
        return 0.0f;
    }
    else if (d < 0.0f) {
        return d + r;
    }
    return d - r;
}

bool frustum_intersects_point(Frustum f, Point3D p) {
    for (int i = 0; i < 6; ++i) {
        vec3 normal = f.planes[i].normal;
        float dist = f.planes[i].distance;
        float side = vec3_dot(p, normal) + dist;
        if (side < 0.0f) {
            return false;
        }
    }
    return true;
}

bool frustum_intersects_sphere(Frustum f, Sphere s) {
    for (int i = 0; i < 6; ++i) {
        vec3 normal = f.planes[i].normal;
        float dist = f.planes[i].distance;
        float side = vec3_dot(s.position, normal) + dist;
        if (side < -s.radius) {
            return false;
        }
    }
    return true;
}

bool frustum_intersects_aabb(Frustum f, AABB aabb) {
    for (int i = 0; i < 6; ++i) {
        float side = classify_aabb(aabb, f.planes[i]);
        if (side < 0) {
            return false;
        }
    }
    return true;
}

bool frustum_intersects_obb(Frustum f, OBB obb) {
    for (int i = 0; i < 6; ++i) {
        float side = classify_obb(obb, f.planes[i]);
        if (side < 0) {
            return false;
        }
    }
    return true;
}

/*******************************************************************************
 * Unprojection / Picking
 ******************************************************************************/

static void mat4_multiply_array(
    float *out,
    const float *in,
    int rows_a,
    int cols_a,
    const float *mat_b,
    int rows_b,
    int cols_b)
{
    (void)rows_b;
    // Multiply a row-major (rows_a x cols_a) array by a row-major
    // (rows_b x cols_b) matrix: out = in * mat_b.
    for (int r = 0; r < rows_a; ++r) {
        for (int c = 0; c < cols_b; ++c) {
            float sum = 0.0f;
            for (int k = 0; k < cols_a; ++k) {
                sum += in[r * cols_a + k] * mat_b[k * cols_b + c];
            }
            out[r * cols_b + c] = sum;
        }
    }
}

vec3 unproject(vec3 viewport_point, vec2 viewport_origin, vec2 viewport_size,
               mat4 view, mat4 projection) {
    /* Step 1: Normalize the input vector to the viewport */
    float normalized[4] = {
        (viewport_point.x - viewport_origin.x) / viewport_size.x,
        (viewport_point.y - viewport_origin.y) / viewport_size.y,
        viewport_point.z,
        1.0f
    };

    /* Step 2: Translate into NDC space */
    float ndc_space[4] = {
        normalized[0], normalized[1],
        normalized[2], normalized[3]
    };
    ndc_space[0] = ndc_space[0] * 2.0f - 1.0f;
    ndc_space[1] = 1.0f - ndc_space[1] * 2.0f;
    if (ndc_space[2] < 0.0f) ndc_space[2] = 0.0f;
    if (ndc_space[2] > 1.0f) ndc_space[2] = 1.0f;

    /* Step 3: NDC to Eye Space */
    mat4 inv_projection = mat4_inverse(projection);
    float eye_space[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    mat4_multiply_array(eye_space, ndc_space, 1, 4, inv_projection.m[0], 4, 4);

    /* Step 4: Eye Space to World Space */
    mat4 inv_view = mat4_inverse(view);
    float world_space[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    mat4_multiply_array(world_space, eye_space, 1, 4, inv_view.m[0], 4, 4);

    /* Step 5: Undo perspective divide */
    if (!CMP(world_space[3], 0.0f)) {
        world_space[0] /= world_space[3];
        world_space[1] /= world_space[3];
        world_space[2] /= world_space[3];
    }

    return (vec3){world_space[0], world_space[1], world_space[2]};
}

Ray3D get_pick_ray(vec2 viewport_point, vec2 viewport_origin, vec2 viewport_size,
                   mat4 view, mat4 projection) {
    vec3 near_point = {viewport_point.x, viewport_point.y, 0.0f};
    vec3 far_point = {viewport_point.x, viewport_point.y, 1.0f};

    vec3 p_near = unproject(near_point, viewport_origin, viewport_size, view, projection);
    vec3 p_far = unproject(far_point, viewport_origin, viewport_size, view, projection);

    vec3 normal = vec3_normalized(vec3_sub(p_far, p_near));
    vec3 origin = p_near;

    return ray3d_create(origin, normal);
}

/*******************************************************************************
 * OBB Helper Functions for Collision
 ******************************************************************************/

void obb_get_vertices(OBB obb, vec3* out_vertices) {
    vec3 C = obb.position;
    vec3 E = obb.size;
    vec3 A[3] = {
        {obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]},
        {obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]},
        {obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]}
    };

    out_vertices[0] = vec3_add(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[1] = vec3_add(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[2] = vec3_add(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[3] = vec3_sub(vec3_add(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[4] = vec3_sub(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[5] = vec3_sub(vec3_sub(vec3_add(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[6] = vec3_sub(vec3_add(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
    out_vertices[7] = vec3_add(vec3_sub(vec3_sub(C, vec3_scale(A[0], E.v[0])), vec3_scale(A[1], E.v[1])), vec3_scale(A[2], E.v[2]));
}

void obb_get_edges(OBB obb, Line3D* out_edges) {
    vec3 v[8];
    obb_get_vertices(obb, v);

    int index[12][2] = {
        {6, 1}, {6, 3}, {6, 4}, {2, 7}, {2, 5}, {2, 0},
        {0, 1}, {0, 3}, {7, 1}, {7, 4}, {4, 5}, {5, 3}
    };

    for (int j = 0; j < 12; ++j) {
        out_edges[j] = line3d_create(v[index[j][0]], v[index[j][1]]);
    }
}

void obb_get_planes(OBB obb, Plane* out_planes) {
    vec3 c = obb.position;
    vec3 e = obb.size;
    vec3 a[3] = {
        {obb.orientation.m[0][0], obb.orientation.m[0][1], obb.orientation.m[0][2]},
        {obb.orientation.m[1][0], obb.orientation.m[1][1], obb.orientation.m[1][2]},
        {obb.orientation.m[2][0], obb.orientation.m[2][1], obb.orientation.m[2][2]}
    };

    out_planes[0] = plane_create(a[0], vec3_dot(a[0], vec3_add(c, vec3_scale(a[0], e.x))));
    out_planes[1] = plane_create(vec3_scale(a[0], -1.0f), -vec3_dot(a[0], vec3_sub(c, vec3_scale(a[0], e.x))));
    out_planes[2] = plane_create(a[1], vec3_dot(a[1], vec3_add(c, vec3_scale(a[1], e.y))));
    out_planes[3] = plane_create(vec3_scale(a[1], -1.0f), -vec3_dot(a[1], vec3_sub(c, vec3_scale(a[1], e.y))));
    out_planes[4] = plane_create(a[2], vec3_dot(a[2], vec3_add(c, vec3_scale(a[2], e.z))));
    out_planes[5] = plane_create(vec3_scale(a[2], -1.0f), -vec3_dot(a[2], vec3_sub(c, vec3_scale(a[2], e.z))));
}

bool clip_to_plane(Plane plane, Line3D line, Point3D* out_point) {
    vec3 ab = vec3_sub(line.end, line.start);

    float n_a = vec3_dot(plane.normal, line.start);
    float n_ab = vec3_dot(plane.normal, ab);

    if (CMP(n_ab, 0.0f)) {
        return false;
    }

    float t = (plane.distance - n_a) / n_ab;
    if (t >= 0.0f && t <= 1.0f) {
        if (out_point != NULL) {
            *out_point = vec3_add(line.start, vec3_scale(ab, t));
        }
        return true;
    }
    return false;
}

int clip_edges_to_obb(const Line3D* edges, int num_edges, OBB obb,
                      Point3D* out_points, int max_points) {
    int count = 0;
    Point3D intersection;

    Plane planes[6];
    obb_get_planes(obb, planes);

    for (int i = 0; i < 6 && count < max_points; ++i) {
        for (int j = 0; j < num_edges && count < max_points; ++j) {
            if (clip_to_plane(planes[i], edges[j], &intersection)) {
                if (point_in_obb(intersection, obb)) {
                    out_points[count++] = intersection;
                }
            }
        }
    }
    return count;
}

float penetration_depth(OBB o1, OBB o2, vec3 axis, bool* out_should_flip) {
    Interval3D i1 = interval3d_from_obb(o1, vec3_normalized(axis));
    Interval3D i2 = interval3d_from_obb(o2, vec3_normalized(axis));

    if (!((i2.min <= i1.max) && (i1.min <= i2.max))) {
        return 0.0f;
    }

    float len1 = i1.max - i1.min;
    float len2 = i2.max - i2.min;
    float min_val = fminf(i1.min, i2.min);
    float max_val = fmaxf(i1.max, i2.max);
    float length = max_val - min_val;

    if (out_should_flip != NULL) {
        *out_should_flip = (i2.min < i1.min);
    }

    return (len1 + len2) - length;
}

/*******************************************************************************
 * Collision Manifold Functions (Chapter 15)
 ******************************************************************************/

CollisionManifold find_collision_features_sphere_sphere(Sphere A, Sphere B) {
    CollisionManifold result;
    collision_manifold_init(&result);

    float r = A.radius + B.radius;
    vec3 d = vec3_sub(B.position, A.position);

    if (vec3_magnitude_sq(d) - r * r > 0 || vec3_magnitude_sq(d) == 0.0f) {
        return result;
    }
    d = vec3_normalized(d);

    result.colliding = true;
    result.normal = d;
    result.depth = fabsf(vec3_magnitude(d) - r) * 0.5f;

    float dtp = A.radius - result.depth;
    Point3D contact = vec3_add(A.position, vec3_scale(d, dtp));

    contact_array_push(&result.contacts, contact);

    return result;
}

CollisionManifold find_collision_features_obb_sphere(OBB A, Sphere B) {
    CollisionManifold result;
    collision_manifold_init(&result);

    Point3D closest_point = closest_point_on_obb(A, B.position);

    float distance_sq = vec3_magnitude_sq(vec3_sub(closest_point, B.position));
    if (distance_sq > B.radius * B.radius) {
        return result;
    }

    vec3 normal;
    if (CMP(distance_sq, 0.0f)) {
        if (CMP(vec3_magnitude_sq(vec3_sub(closest_point, A.position)), 0.0f)) {
            return result;
        }
        normal = vec3_normalized(vec3_sub(closest_point, A.position));
    }
    else {
        normal = vec3_normalized(vec3_sub(B.position, closest_point));
    }

    Point3D outside_point = vec3_sub(B.position, vec3_scale(normal, B.radius));
    float distance = vec3_magnitude(vec3_sub(closest_point, outside_point));

    result.colliding = true;
    contact_array_push(&result.contacts, 
        vec3_add(closest_point, vec3_scale(vec3_sub(outside_point, closest_point), 0.5f)));
    result.normal = normal;
    result.depth = distance * 0.5f;

    return result;
}

CollisionManifold find_collision_features_obb_obb(OBB A, OBB B) {
    CollisionManifold result;
    collision_manifold_init(&result);

    /* Early out with bounding sphere test */
    Sphere s1 = sphere_create(A.position, vec3_magnitude(A.size));
    Sphere s2 = sphere_create(B.position, vec3_magnitude(B.size));

    if (!sphere_sphere(s1, s2)) {
        return result;
    }

    vec3 test[15] = {
        {A.orientation.m[0][0], A.orientation.m[0][1], A.orientation.m[0][2]},
        {A.orientation.m[1][0], A.orientation.m[1][1], A.orientation.m[1][2]},
        {A.orientation.m[2][0], A.orientation.m[2][1], A.orientation.m[2][2]},
        {B.orientation.m[0][0], B.orientation.m[0][1], B.orientation.m[0][2]},
        {B.orientation.m[1][0], B.orientation.m[1][1], B.orientation.m[1][2]},
        {B.orientation.m[2][0], B.orientation.m[2][1], B.orientation.m[2][2]}
    };

    for (int i = 0; i < 3; ++i) {
        test[6 + i * 3 + 0] = vec3_cross(test[i], test[3]);
        test[6 + i * 3 + 1] = vec3_cross(test[i], test[4]);
        test[6 + i * 3 + 2] = vec3_cross(test[i], test[5]);
    }

    vec3* hit_normal = NULL;
    bool should_flip;

    for (int i = 0; i < 15; ++i) {
        if (test[i].x < 0.000001f) test[i].x = 0.0f;
        if (test[i].y < 0.000001f) test[i].y = 0.0f;
        if (test[i].z < 0.000001f) test[i].z = 0.0f;
        if (vec3_magnitude_sq(test[i]) < 0.001f) {
            continue;
        }

        float depth = penetration_depth(A, B, test[i], &should_flip);
        if (depth <= 0.0f) {
            return result;
        }
        else if (depth < result.depth) {
            if (should_flip) {
                test[i] = vec3_scale(test[i], -1.0f);
            }
            result.depth = depth;
            hit_normal = &test[i];
        }
    }

    if (hit_normal == NULL) {
        return result;
    }
    vec3 axis = vec3_normalized(*hit_normal);

    /* Clip edges */
    Line3D edges_b[12], edges_a[12];
    obb_get_edges(B, edges_b);
    obb_get_edges(A, edges_a);

    Point3D clip_buffer[72];  /* 12 edges * 6 planes max */
    int c1_count = clip_edges_to_obb(edges_b, 12, A, clip_buffer, 36);
    int c2_count = clip_edges_to_obb(edges_a, 12, B, clip_buffer + c1_count, 36);

    contact_array_reserve(&result.contacts, c1_count + c2_count);
    for (int i = 0; i < c1_count + c2_count; ++i) {
        contact_array_push(&result.contacts, clip_buffer[i]);
    }

    Interval3D interval = interval3d_from_obb(A, axis);
    float distance = (interval.max - interval.min) * 0.5f - result.depth * 0.5f;
    vec3 point_on_plane = vec3_add(A.position, vec3_scale(axis, distance));

    /* Project contacts onto collision plane and remove duplicates */
    for (int i = result.contacts.count - 1; i >= 0; --i) {
        vec3 contact = result.contacts.data[i];
        result.contacts.data[i] = vec3_add(contact, 
            vec3_scale(axis, vec3_dot(axis, vec3_sub(point_on_plane, contact))));

        for (int j = result.contacts.count - 1; j > i; --j) {
            if (vec3_magnitude_sq(vec3_sub(result.contacts.data[j], result.contacts.data[i])) < 0.0001f) {
                contact_array_erase(&result.contacts, j);
                break;
            }
        }
    }

    result.colliding = true;
    result.normal = axis;

    return result;
}