/**
 * geometry3d.h - 3D Geometry Primitives and Collision Detection (C23)
 * 
 * Translated from C++ to C23
 * Original: Geometry3D.h/.cpp
 */

#ifndef GEOMETRY3D_H
#define GEOMETRY3D_H

#include "vectors.h"
#include "matrices.h"

#include <stdbool.h>
#include <stdio.h>

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef vec3 Point3D;

typedef struct Line3D {
    Point3D start;
    Point3D end;
} Line3D;

typedef struct Ray3D {
    Point3D origin;
    vec3    direction;  /* Should be normalized */
} Ray3D;

typedef struct Sphere {
    Point3D position;
    float   radius;
} Sphere;

typedef struct AABB {
    Point3D position;   /* Center */
    vec3    size;       /* HALF SIZE */
} AABB;

typedef struct OBB {
    Point3D position;   /* Center */
    vec3    size;       /* HALF SIZE */
    mat3    orientation;
} OBB;

typedef struct Plane {
    vec3  normal;
    float distance;
} Plane;

typedef struct Triangle {
    union {
        struct {
            Point3D a;
            Point3D b;
            Point3D c;
        };
#ifndef NO_EXTRAS
        struct {
            Point3D p1;
            Point3D p2;
            Point3D p3;
        };
#endif
        Point3D points[3];
        float   values[9];
    };
} Triangle;

typedef struct Interval3D {
    float min;
    float max;
} Interval3D;

typedef struct Frustum {
    union {
        struct {
            Plane top;
            Plane bottom;
            Plane left;
            Plane right;
            Plane near_plane;   /* Renamed from _near to avoid reserved word issues */
            Plane far_plane;    /* Renamed from _far to avoid reserved word issues */
        };
        Plane planes[6];
    };
} Frustum;

typedef struct RaycastResult {
    vec3  point;
    vec3  normal;
    float t;
    bool  hit;
} RaycastResult;

typedef struct BVHNode {
    AABB            bounds;
    struct BVHNode* children;       /* Array of 8 children or NULL */
    int             num_triangles;
    int*            triangles;      /* Indices into mesh triangles */
} BVHNode;

typedef struct Mesh {
    int num_triangles;
    union {
        Triangle* triangles;
        Point3D*  vertices;
        float*    values;
    };
    BVHNode* accelerator;
} Mesh;

typedef struct Model {
    Mesh*         content;
    AABB          bounds;
    vec3          position;
    vec3          rotation;
    bool          flag;
    struct Model* parent;
} Model;

/* Dynamic array for contact points (replaces std::vector<vec3>) */
typedef struct ContactArray {
    vec3* data;
    int   count;
    int   capacity;
} ContactArray;

typedef struct CollisionManifold {
    bool         colliding;
    vec3         normal;
    float        depth;
    ContactArray contacts;
} CollisionManifold;

/* Dynamic array for Line3D (replaces std::vector<Line>) */
typedef struct Line3DArray {
    Line3D* data;
    int     count;
    int     capacity;
} Line3DArray;

/* Dynamic array for Plane (replaces std::vector<Plane>) */
typedef struct PlaneArray {
    Plane* data;
    int    count;
    int    capacity;
} PlaneArray;

/*******************************************************************************
 * Type Aliases (NO_EXTRAS compatibility)
 ******************************************************************************/

#ifndef NO_EXTRAS
typedef Line3D     Line;
typedef Ray3D      Ray;
typedef AABB       Rectangle3D;
typedef Interval3D Interval;
typedef Point3D    Point;
#endif

/*******************************************************************************
 * Constructors / Initializers
 ******************************************************************************/

static inline Line3D line3d_create(Point3D start, Point3D end) {
    return (Line3D){ .start = start, .end = end };
}

static inline Line3D line3d_default(void) {
    return (Line3D){ .start = {{{0, 0, 0}}}, .end = {{{0, 0, 0}}} };
}

static inline Ray3D ray3d_create(Point3D origin, vec3 direction) {
    return (Ray3D){ .origin = origin, .direction = vec3_normalized(direction) };
}

static inline Ray3D ray3d_default(void) {
    return (Ray3D){ .origin = {{{0, 0, 0}}}, .direction = {{{0, 0, 1}}} };
}

static inline void ray3d_normalize_direction(Ray3D* self) {
    self->direction = vec3_normalized(self->direction);
}

static inline Sphere sphere_create(Point3D position, float radius) {
    return (Sphere){ .position = position, .radius = radius };
}

static inline Sphere sphere_default(void) {
    return (Sphere){ .position = {{{0, 0, 0}}}, .radius = 1.0f };
}

static inline AABB aabb_create(Point3D position, vec3 size) {
    return (AABB){ .position = position, .size = size };
}

static inline AABB aabb_default(void) {
    return (AABB){ .position = {{{0, 0, 0}}}, .size = {{{1, 1, 1}}} };
}

static inline OBB obb_create(Point3D position, vec3 size, mat3 orientation) {
    return (OBB){ .position = position, .size = size, .orientation = orientation };
}

static inline OBB obb_create_simple(Point3D position, vec3 size) {
    return (OBB){ .position = position, .size = size, .orientation = mat3_identity() };
}

static inline OBB obb_default(void) {
    return (OBB){ .position = {{{0, 0, 0}}}, .size = {{{1, 1, 1}}}, .orientation = mat3_identity() };
}

static inline Plane plane_create(vec3 normal, float distance) {
    return (Plane){ .normal = normal, .distance = distance };
}

static inline Plane plane_default(void) {
    return (Plane){ .normal = {{{1, 0, 0}}}, .distance = 0.0f };
}

static inline Triangle triangle_create(Point3D a, Point3D b, Point3D c) {
    Triangle t;
    t.a = a;
    t.b = b;
    t.c = c;
    return t;
}

static inline Triangle triangle_default(void) {
    Triangle t = {0};
    return t;
}

static inline Frustum frustum_default(void) {
    Frustum f = {0};
    return f;
}

static inline BVHNode bvhnode_default(void) {
    return (BVHNode){
        .bounds = aabb_default(),
        .children = NULL,
        .num_triangles = 0,
        .triangles = NULL
    };
}

static inline Mesh mesh_default(void) {
    return (Mesh){
        .num_triangles = 0,
        .triangles = NULL,
        .accelerator = NULL
    };
}

static inline Model model_default(void) {
    return (Model){
        .content = NULL,
        .bounds = aabb_default(),
        .position = {{{0, 0, 0}}},
        .rotation = {{{0, 0, 0}}},
        .flag = false,
        .parent = NULL
    };
}

/*******************************************************************************
 * Dynamic Array Operations
 ******************************************************************************/

/* ContactArray */
void contact_array_init(ContactArray* arr);
void contact_array_free(ContactArray* arr);
void contact_array_push(ContactArray* arr, vec3 point);
void contact_array_clear(ContactArray* arr);
void contact_array_reserve(ContactArray* arr, int capacity);
void contact_array_erase(ContactArray* arr, int index);

/* Line3DArray */
void line3d_array_init(Line3DArray* arr);
void line3d_array_free(Line3DArray* arr);
void line3d_array_push(Line3DArray* arr, Line3D line);
void line3d_array_reserve(Line3DArray* arr, int capacity);

/* PlaneArray */
void plane_array_init(PlaneArray* arr);
void plane_array_free(PlaneArray* arr);
void plane_array_push(PlaneArray* arr, Plane plane);

/*******************************************************************************
 * RaycastResult / CollisionManifold
 ******************************************************************************/

void raycast_result_reset(RaycastResult* result);
void collision_manifold_reset(CollisionManifold* result);
void collision_manifold_init(CollisionManifold* result);
void collision_manifold_free(CollisionManifold* result);

/*******************************************************************************
 * Line3D Operations
 ******************************************************************************/

float line3d_length(Line3D line);
float line3d_length_sq(Line3D line);

/*******************************************************************************
 * Ray3D Operations
 ******************************************************************************/

Ray3D ray3d_from_points(Point3D from, Point3D to);

/*******************************************************************************
 * AABB Operations
 ******************************************************************************/

vec3 aabb_get_min(AABB aabb);
vec3 aabb_get_max(AABB aabb);
AABB aabb_from_min_max(vec3 min, vec3 max);

/*******************************************************************************
 * Plane Operations
 ******************************************************************************/

float plane_equation(Point3D point, Plane plane);
Plane plane_from_triangle(Triangle t);

#ifndef NO_EXTRAS
float plane_equation_reversed(Plane plane, Point3D point);
#endif

/*******************************************************************************
 * Point Containment Tests
 ******************************************************************************/

bool point_in_sphere(Point3D point, Sphere sphere);
bool point_in_aabb(Point3D point, AABB aabb);
bool point_in_obb(Point3D point, OBB obb);
bool point_on_plane(Point3D point, Plane plane);
bool point_on_line3d(Point3D point, Line3D line);
bool point_on_ray3d(Point3D point, Ray3D ray);
bool point_in_triangle(Point3D point, Triangle triangle);

#ifndef NO_EXTRAS
/* Alias functions for convenience */
bool point_in_plane(Point3D point, Plane plane);
bool point_in_line3d(Point3D point, Line3D line);
bool point_in_ray3d(Point3D point, Ray3D ray);

/* ContainsPoint variants (all map to point_in_* functions) */
#define contains_point_sphere_point(sphere, point) point_in_sphere(point, sphere)
#define contains_point_point_sphere(point, sphere) point_in_sphere(point, sphere)
#define contains_point_aabb_point(aabb, point)     point_in_aabb(point, aabb)
#define contains_point_point_aabb(point, aabb)     point_in_aabb(point, aabb)
#define contains_point_obb_point(obb, point)       point_in_obb(point, obb)
#define contains_point_point_obb(point, obb)       point_in_obb(point, obb)
#define contains_point_plane_point(plane, point)   point_on_plane(point, plane)
#define contains_point_point_plane(point, plane)   point_on_plane(point, plane)
#define contains_point_line_point(line, point)     point_on_line3d(point, line)
#define contains_point_point_line(point, line)     point_on_line3d(point, line)
#define contains_point_ray_point(ray, point)       point_on_ray3d(point, ray)
#define contains_point_point_ray(point, ray)       point_on_ray3d(point, ray)
#endif

/*******************************************************************************
 * Closest Point Functions
 ******************************************************************************/

Point3D closest_point_on_sphere(Sphere sphere, Point3D point);
Point3D closest_point_on_aabb(AABB aabb, Point3D point);
Point3D closest_point_on_obb(OBB obb, Point3D point);
Point3D closest_point_on_plane(Plane plane, Point3D point);
Point3D closest_point_on_line3d(Line3D line, Point3D point);
Point3D closest_point_on_ray3d(Ray3D ray, Point3D point);
Point3D closest_point_on_triangle(Triangle triangle, Point3D point);

#ifndef NO_EXTRAS
/* Reversed argument order variants */
#define closest_point_point_sphere(point, sphere)     closest_point_on_sphere(sphere, point)
#define closest_point_point_aabb(point, aabb)         closest_point_on_aabb(aabb, point)
#define closest_point_point_obb(point, obb)           closest_point_on_obb(obb, point)
#define closest_point_point_plane(point, plane)       closest_point_on_plane(plane, point)
#define closest_point_point_line3d(point, line)       closest_point_on_line3d(line, point)
#define closest_point_point_ray3d(point, ray)         closest_point_on_ray3d(ray, point)
#define closest_point_point_triangle(point, triangle) closest_point_on_triangle(triangle, point)
#endif

/*******************************************************************************
 * Interval / SAT Functions
 ******************************************************************************/

Interval3D interval3d_from_aabb(AABB aabb, vec3 axis);
Interval3D interval3d_from_obb(OBB obb, vec3 axis);
Interval3D interval3d_from_triangle(Triangle triangle, vec3 axis);

bool overlap_on_axis_aabb_obb(AABB aabb, OBB obb, vec3 axis);
bool overlap_on_axis_obb_obb(OBB obb1, OBB obb2, vec3 axis);
bool overlap_on_axis_aabb_triangle(AABB aabb, Triangle tri, vec3 axis);
bool overlap_on_axis_obb_triangle(OBB obb, Triangle tri, vec3 axis);
bool overlap_on_axis_triangle_triangle(Triangle t1, Triangle t2, vec3 axis);

/*******************************************************************************
 * Shape-Shape Intersection Tests
 ******************************************************************************/

bool sphere_sphere(Sphere s1, Sphere s2);
bool sphere_aabb(Sphere sphere, AABB aabb);
bool sphere_obb(Sphere sphere, OBB obb);
bool sphere_plane(Sphere sphere, Plane plane);

bool aabb_aabb(AABB a1, AABB a2);
bool aabb_obb(AABB aabb, OBB obb);
bool aabb_plane(AABB aabb, Plane plane);

bool obb_obb(OBB o1, OBB o2);
bool obb_plane(OBB obb, Plane plane);

bool plane_plane(Plane p1, Plane p2);

bool triangle_sphere(Triangle t, Sphere s);
bool triangle_aabb(Triangle t, AABB aabb);
bool triangle_obb(Triangle t, OBB obb);
bool triangle_plane(Triangle t, Plane p);
bool triangle_triangle(Triangle t1, Triangle t2);
bool triangle_triangle_robust(Triangle t1, Triangle t2);

/* Argument order swap macros */
#define aabb_sphere(aabb, sphere)     sphere_aabb(sphere, aabb)
#define obb_sphere(obb, sphere)       sphere_obb(sphere, obb)
#define plane_sphere(plane, sphere)   sphere_plane(sphere, plane)
#define obb_aabb(obb, aabb)           aabb_obb(aabb, obb)
#define plane_aabb(plane, aabb)       aabb_plane(aabb, plane)
#define plane_obb(plane, obb)         obb_plane(obb, plane)

#define sphere_triangle(s, t)         triangle_sphere(t, s)
#define aabb_triangle(a, t)           triangle_aabb(t, a)
#define obb_triangle(o, t)            triangle_obb(t, o)
#define plane_triangle(p, t)          triangle_plane(t, p)

/*******************************************************************************
 * Raycasting
 ******************************************************************************/

bool raycast_sphere(Sphere sphere, Ray3D ray, RaycastResult* out_result);
bool raycast_aabb(AABB aabb, Ray3D ray, RaycastResult* out_result);
bool raycast_obb(OBB obb, Ray3D ray, RaycastResult* out_result);
bool raycast_plane(Plane plane, Ray3D ray, RaycastResult* out_result);
bool raycast_triangle(Triangle triangle, Ray3D ray, RaycastResult* out_result);

#ifndef NO_EXTRAS
/* Reversed argument order variants */
#define raycast_ray_sphere(ray, sphere, result) raycast_sphere(sphere, ray, result)
#define raycast_ray_aabb(ray, aabb, result)     raycast_aabb(aabb, ray, result)
#define raycast_ray_obb(ray, obb, result)       raycast_obb(obb, ray, result)
#define raycast_ray_plane(ray, plane, result)   raycast_plane(plane, ray, result)
#endif

/*******************************************************************************
 * Line Tests
 ******************************************************************************/

bool linetest_sphere(Sphere sphere, Line3D line);
bool linetest_aabb(AABB aabb, Line3D line);
bool linetest_obb(OBB obb, Line3D line);
bool linetest_plane(Plane plane, Line3D line);
bool linetest_triangle(Triangle triangle, Line3D line);

#ifndef NO_EXTRAS
/* Reversed argument order variants */
#define linetest_line_sphere(line, sphere) linetest_sphere(sphere, line)
#define linetest_line_aabb(line, aabb)     linetest_aabb(aabb, line)
#define linetest_line_obb(line, obb)       linetest_obb(obb, line)
#define linetest_line_plane(line, plane)   linetest_plane(plane, line)
#endif

/*******************************************************************************
 * Triangle Utilities
 ******************************************************************************/

vec3 barycentric(Point3D p, Triangle t);
vec3 sat_cross_edge(vec3 a, vec3 b, vec3 c, vec3 d);

#ifndef NO_EXTRAS
vec3 barycentric_optimized(Point3D p, Triangle t);
vec3 triangle_centroid(Triangle t);
#endif

/*******************************************************************************
 * BVH / Mesh Operations
 ******************************************************************************/

void mesh_accelerate(Mesh* mesh);
void bvhnode_split(BVHNode* node, const Mesh* mesh, int depth);
void bvhnode_free(BVHNode* node);

bool  linetest_mesh(const Mesh* mesh, Line3D line);
bool  mesh_sphere(const Mesh* mesh, Sphere sphere);
bool  mesh_aabb(const Mesh* mesh, AABB aabb);
bool  mesh_obb(const Mesh* mesh, OBB obb);
bool  mesh_plane(const Mesh* mesh, Plane plane);
bool  mesh_triangle(const Mesh* mesh, Triangle triangle);
float mesh_ray(const Mesh* mesh, Ray3D ray);

#ifndef NO_EXTRAS
float raycast_mesh(const Mesh* mesh, Ray3D ray);
float raycast_model(const Model* model, Ray3D ray);
#endif

/*******************************************************************************
 * Model Operations
 ******************************************************************************/

void  model_set_content(Model* model, Mesh* mesh);
Mesh* model_get_mesh(const Model* model);
AABB  model_get_bounds(const Model* model);
mat4  model_get_world_matrix(const Model* model);
OBB   model_get_obb(const Model* model);

float model_ray(const Model* model, Ray3D ray);
bool  linetest_model(const Model* model, Line3D line);
bool  model_sphere(const Model* model, Sphere sphere);
bool  model_aabb(const Model* model, AABB aabb);
bool  model_obb(const Model* model, OBB obb);
bool  model_plane(const Model* model, Plane plane);
bool  model_triangle(const Model* model, Triangle triangle);

/*******************************************************************************
 * Frustum Operations
 ******************************************************************************/

Point3D plane_intersection(Plane p1, Plane p2, Plane p3);
void    frustum_get_corners(Frustum f, vec3* out_corners);  /* out_corners[8] */

float classify_aabb(AABB aabb, Plane plane);
float classify_obb(OBB obb, Plane plane);

bool frustum_intersects_point(Frustum f, Point3D p);
bool frustum_intersects_sphere(Frustum f, Sphere s);
bool frustum_intersects_aabb(Frustum f, AABB aabb);
bool frustum_intersects_obb(Frustum f, OBB obb);

/*******************************************************************************
 * Unprojection / Picking
 ******************************************************************************/

vec3  unproject(vec3 viewport_point, vec2 viewport_origin, vec2 viewport_size,
                mat4 view, mat4 projection);
Ray3D get_pick_ray(vec2 viewport_point, vec2 viewport_origin, vec2 viewport_size,
                   mat4 view, mat4 projection);

/*******************************************************************************
 * Collision Manifold Functions (Chapter 15)
 ******************************************************************************/

/* OBB helper functions - output to fixed-size arrays */
void obb_get_vertices(OBB obb, vec3* out_vertices);      /* out_vertices[8] */
void obb_get_edges(OBB obb, Line3D* out_edges);          /* out_edges[12] */
void obb_get_planes(OBB obb, Plane* out_planes);         /* out_planes[6] */

bool  clip_to_plane(Plane plane, Line3D line, Point3D* out_point);
int   clip_edges_to_obb(const Line3D* edges, int num_edges, OBB obb,
                        Point3D* out_points, int max_points);
float penetration_depth(OBB o1, OBB o2, vec3 axis, bool* out_should_flip);

CollisionManifold find_collision_features_sphere_sphere(Sphere a, Sphere b);
CollisionManifold find_collision_features_obb_sphere(OBB a, Sphere b);
CollisionManifold find_collision_features_obb_obb(OBB a, OBB b);

/*******************************************************************************
 * Debug Print Functions
 ******************************************************************************/

#ifndef NO_EXTRAS
void line3d_print(FILE* stream, Line3D shape);
void ray3d_print(FILE* stream, Ray3D shape);
void sphere_print(FILE* stream, Sphere shape);
void aabb_print(FILE* stream, AABB shape);
void obb_print(FILE* stream, OBB shape);
void plane_print(FILE* stream, Plane shape);
void triangle_print(FILE* stream, Triangle shape);
#endif

#endif /* GEOMETRY3D_H */