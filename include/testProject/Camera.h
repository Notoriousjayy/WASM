#ifndef CAMERA_H
#define CAMERA_H

#include "matrices.h"
#include "geometry3d.h"

#include <stdbool.h>

/*******************************************************************************
 * Camera - Base camera type with projection and view matrix management
 ******************************************************************************/

typedef struct Camera {
    float fov;
    float aspect;
    float near_plane;
    float far_plane;
    float width;
    float height;

    mat4 world_matrix;   // World transform
    mat4 proj_matrix;    // Projection matrix
    int  projection_mode; // 0 = Perspective, 1 = Ortho, 2 = User-defined
} Camera;

// Initialization
Camera camera_create(void);
Camera camera_create_perspective(float field_of_view, float aspect_ratio, 
                                  float near_plane, float far_plane);
Camera camera_create_orthographic(float width, float height,
                                   float near_plane, float far_plane);

// Matrix access
mat4 camera_get_world_matrix(const Camera* self);
mat4 camera_get_view_matrix(Camera* self);  // May ortho-normalize internally
mat4 camera_get_projection_matrix(const Camera* self);

// Properties
float camera_get_aspect(const Camera* self);
bool  camera_is_orthographic(const Camera* self);
bool  camera_is_perspective(const Camera* self);

// Orthonormalization
bool camera_is_orthonormal(const Camera* self);
void camera_orthonormalize(Camera* self);

// Configuration
void camera_resize(Camera* self, int width, int height);
void camera_set_perspective(Camera* self, float fov, float aspect, 
                            float z_near, float z_far);
void camera_set_orthographic(Camera* self, float width, float height,
                             float z_near, float z_far);
void camera_set_projection(Camera* self, mat4 projection);
void camera_set_world(Camera* self, mat4 world);

// Frustum extraction
Frustum camera_get_frustum(Camera* self);


/*******************************************************************************
 * OrbitCamera - Camera that orbits around a target point
 ******************************************************************************/

typedef struct OrbitCamera {
    Camera base;  // "Inherits" from Camera via composition

    vec3 target;
    vec2 pan_speed;

    float zoom_distance;
    vec2  zoom_distance_limit;  // x = min, y = max
    float zoom_speed;

    vec2 rotation_speed;
    vec2 y_rotation_limit;      // x = min, y = max
    vec2 current_rotation;
} OrbitCamera;

// Initialization
OrbitCamera orbit_camera_create(void);

// Controls
void orbit_camera_rotate(OrbitCamera* self, vec2 delta_rot, float delta_time);
void orbit_camera_zoom(OrbitCamera* self, float delta_zoom, float delta_time);
void orbit_camera_pan(OrbitCamera* self, vec2 delta_pan, float delta_time);

// Update (call each frame)
void orbit_camera_update(OrbitCamera* self, float delta_time);

// Setters
void orbit_camera_set_target(OrbitCamera* self, vec3 new_target);
void orbit_camera_set_zoom(OrbitCamera* self, float zoom);
void orbit_camera_set_rotation(OrbitCamera* self, vec2 rotation);

// Utility
float orbit_camera_clamp_angle(float angle, float min, float max);
void  orbit_camera_print_debug(const OrbitCamera* self);

/*******************************************************************************
 * Convenience: Access base Camera from OrbitCamera
 ******************************************************************************/

static inline Camera* orbit_camera_as_camera(OrbitCamera* self) {
    return &self->base;
}

static inline const Camera* orbit_camera_as_camera_const(const OrbitCamera* self) {
    return &self->base;
}

#endif // CAMERA_H