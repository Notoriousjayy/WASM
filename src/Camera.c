#include "camera.h"
#include "compare.h"  // For CMP macro

#include <stdio.h>
#include <math.h>

/*******************************************************************************
 * Camera Implementation
 ******************************************************************************/

Camera camera_create(void) {
    Camera cam = {
        .fov            = 60.0f,
        .aspect         = 1.3f,
        .near_plane     = 0.01f,
        .far_plane      = 1000.0f,
        .width          = 1.0f,
        .height         = 1.0f,
        .world_matrix   = mat4_identity(),
        .proj_matrix    = mat4_projection(60.0f, 1.3f, 0.01f, 1000.0f),
        .projection_mode = 0
    };
    return cam;
}

Camera camera_create_perspective(float field_of_view, float aspect_ratio,
                                  float near_plane, float far_plane) {
    Camera cam = camera_create();
    camera_set_perspective(&cam, field_of_view, aspect_ratio, near_plane, far_plane);
    return cam;
}

Camera camera_create_orthographic(float width, float height,
                                   float near_plane, float far_plane) {
    Camera cam = camera_create();
    camera_set_orthographic(&cam, width, height, near_plane, far_plane);
    return cam;
}

mat4 camera_get_world_matrix(const Camera* self) {
    return self->world_matrix;
}

bool camera_is_orthonormal(const Camera* self) {
    vec3 right   = (vec3){ self->world_matrix.m[0][0], 
                           self->world_matrix.m[0][1], 
                           self->world_matrix.m[0][2] };
    vec3 up      = (vec3){ self->world_matrix.m[1][0], 
                           self->world_matrix.m[1][1], 
                           self->world_matrix.m[1][2] };
    vec3 forward = (vec3){ self->world_matrix.m[2][0], 
                           self->world_matrix.m[2][1], 
                           self->world_matrix.m[2][2] };

    // Check each axis is unit length
    if (!CMP(vec3_dot(right, right), 1.0f)) return false;
    if (!CMP(vec3_dot(up, up), 1.0f)) return false;
    if (!CMP(vec3_dot(forward, forward), 1.0f)) return false;

    // Check axes are perpendicular
    if (!CMP(vec3_dot(forward, up), 0.0f)) return false;
    if (!CMP(vec3_dot(forward, right), 0.0f)) return false;
    if (!CMP(vec3_dot(right, up), 0.0f)) return false;

    return true;
}

void camera_orthonormalize(Camera* self) {
    vec3 right   = (vec3){ self->world_matrix.m[0][0], 
                           self->world_matrix.m[0][1], 
                           self->world_matrix.m[0][2] };
    vec3 up      = (vec3){ self->world_matrix.m[1][0], 
                           self->world_matrix.m[1][1], 
                           self->world_matrix.m[1][2] };
    vec3 forward = (vec3){ self->world_matrix.m[2][0], 
                           self->world_matrix.m[2][1], 
                           self->world_matrix.m[2][2] };

    vec3 f = vec3_normalized(forward);
    vec3 r = vec3_normalized(vec3_cross(up, f));
    vec3 u = vec3_cross(f, r);

    self->world_matrix = (mat4){
        .m = {
            { r.x, r.y, r.z, 0.0f },
            { u.x, u.y, u.z, 0.0f },
            { f.x, f.y, f.z, 0.0f },
            { self->world_matrix.m[3][0], 
              self->world_matrix.m[3][1], 
              self->world_matrix.m[3][2], 1.0f }
        }
    };
}

mat4 camera_get_view_matrix(Camera* self) {
    if (!camera_is_orthonormal(self)) {
        camera_orthonormalize(self);
    }

    mat4 inverse = mat4_transpose(self->world_matrix);
    inverse.m[3][0] = inverse.m[0][3] = 0.0f;
    inverse.m[3][1] = inverse.m[1][3] = 0.0f;
    inverse.m[3][2] = inverse.m[2][3] = 0.0f;

    vec3 right    = (vec3){ self->world_matrix.m[0][0], 
                            self->world_matrix.m[0][1], 
                            self->world_matrix.m[0][2] };
    vec3 up       = (vec3){ self->world_matrix.m[1][0], 
                            self->world_matrix.m[1][1], 
                            self->world_matrix.m[1][2] };
    vec3 forward  = (vec3){ self->world_matrix.m[2][0], 
                            self->world_matrix.m[2][1], 
                            self->world_matrix.m[2][2] };
    vec3 position = (vec3){ self->world_matrix.m[3][0], 
                            self->world_matrix.m[3][1], 
                            self->world_matrix.m[3][2] };

    inverse.m[3][0] = -vec3_dot(right, position);
    inverse.m[3][1] = -vec3_dot(up, position);
    inverse.m[3][2] = -vec3_dot(forward, position);

    return inverse;
}

float camera_get_aspect(const Camera* self) {
    return self->aspect;
}

mat4 camera_get_projection_matrix(const Camera* self) {
    return self->proj_matrix;
}

void camera_resize(Camera* self, int width, int height) {
    self->aspect = (float)width / (float)height;

    if (self->projection_mode == 0) {  // Perspective
        self->proj_matrix = mat4_projection(self->fov, self->aspect, 
                                            self->near_plane, self->far_plane);
    } else if (self->projection_mode == 1) {  // Ortho
        self->width = (float)width;
        self->height = (float)height;

        float half_w = self->width * 0.5f;
        float half_h = self->height * 0.5f;

        self->proj_matrix = mat4_ortho(-half_w, half_w, half_h, -half_h,
                                       self->near_plane, self->far_plane);
    }
    // projection_mode == 2: User-defined, don't modify
}

bool camera_is_orthographic(const Camera* self) {
    return self->projection_mode == 1;
}

bool camera_is_perspective(const Camera* self) {
    return self->projection_mode == 0;
}

void camera_set_perspective(Camera* self, float fov, float aspect,
                            float z_near, float z_far) {
    self->fov = fov;
    self->aspect = aspect;
    self->near_plane = z_near;
    self->far_plane = z_far;
    self->proj_matrix = mat4_projection(fov, aspect, z_near, z_far);
    self->projection_mode = 0;
}

void camera_set_orthographic(Camera* self, float width, float height,
                             float z_near, float z_far) {
    self->width = width;
    self->height = height;
    self->near_plane = z_near;
    self->far_plane = z_far;

    float half_w = width * 0.5f;
    float half_h = height * 0.5f;

    self->proj_matrix = mat4_ortho(-half_w, half_w, half_h, -half_h, z_near, z_far);
    self->projection_mode = 1;
}

void camera_set_projection(Camera* self, mat4 projection) {
    self->proj_matrix = projection;
    self->projection_mode = 2;
}

void camera_set_world(Camera* self, mat4 world) {
    self->world_matrix = world;
}

Frustum camera_get_frustum(Camera* self) {
    Frustum result = {0};

    mat4 vp = mat4_mul(camera_get_view_matrix(self), 
                       camera_get_projection_matrix(self));

    vec3 col1 = (vec3){ vp.m[0][0], vp.m[1][0], vp.m[2][0] };
    vec3 col2 = (vec3){ vp.m[0][1], vp.m[1][1], vp.m[2][1] };
    vec3 col3 = (vec3){ vp.m[0][2], vp.m[1][2], vp.m[2][2] };
    vec3 col4 = (vec3){ vp.m[0][3], vp.m[1][3], vp.m[2][3] };

    // Plane normals
    result.left.normal   = vec3_add(col4, col1);
    result.right.normal  = vec3_sub(col4, col1);
    result.bottom.normal = vec3_add(col4, col2);
    result.top.normal    = vec3_sub(col4, col2);
    result._near.normal  = col3;
    result._far.normal   = vec3_sub(col4, col3);

    // Plane distances
    result.left.distance   = vp.m[3][3] + vp.m[3][0];
    result.right.distance  = vp.m[3][3] - vp.m[3][0];
    result.bottom.distance = vp.m[3][3] + vp.m[3][1];
    result.top.distance    = vp.m[3][3] - vp.m[3][1];
    result._near.distance  = vp.m[3][2];
    result._far.distance   = vp.m[3][3] - vp.m[3][2];

    // Normalize all 6 planes
    for (int i = 0; i < 6; ++i) {
        float mag = 1.0f / vec3_magnitude(result.planes[i].normal);
        result.planes[i].normal = vec3_normalized(result.planes[i].normal);
        result.planes[i].distance *= mag;
    }

    return result;
}

/*******************************************************************************
 * OrbitCamera Implementation
 ******************************************************************************/

OrbitCamera orbit_camera_create(void) {
    OrbitCamera cam = {
        .base = camera_create(),
        .target = (vec3){0, 0, 0},
        .zoom_distance = 10.0f,
        .zoom_speed = 200.0f,
        .rotation_speed = (vec2){250.0f, 120.0f},
        .y_rotation_limit = (vec2){-20.0f, 80.0f},
        .zoom_distance_limit = (vec2){3.0f, 15.0f},
        .current_rotation = (vec2){0, 0},
        .pan_speed = (vec2){180.0f, 180.0f}
    };
    return cam;
}

float orbit_camera_clamp_angle(float angle, float min, float max) {
    while (angle < -360.0f) angle += 360.0f;
    while (angle >  360.0f) angle -= 360.0f;
    if (angle < min) angle = min;
    if (angle > max) angle = max;
    return angle;
}

void orbit_camera_rotate(OrbitCamera* self, vec2 delta_rot, float delta_time) {
    self->current_rotation.x += delta_rot.x * self->rotation_speed.x 
                                * self->zoom_distance * delta_time;
    self->current_rotation.y += delta_rot.y * self->rotation_speed.y 
                                * self->zoom_distance * delta_time;

    self->current_rotation.x = orbit_camera_clamp_angle(
        self->current_rotation.x, -360.0f, 360.0f);
    self->current_rotation.y = orbit_camera_clamp_angle(
        self->current_rotation.y, 
        self->y_rotation_limit.x, 
        self->y_rotation_limit.y);
}

void orbit_camera_zoom(OrbitCamera* self, float delta_zoom, float delta_time) {
    self->zoom_distance += delta_zoom * self->zoom_speed * delta_time;
    
    if (self->zoom_distance < self->zoom_distance_limit.x) {
        self->zoom_distance = self->zoom_distance_limit.x;
    }
    if (self->zoom_distance > self->zoom_distance_limit.y) {
        self->zoom_distance = self->zoom_distance_limit.y;
    }
}

void orbit_camera_pan(OrbitCamera* self, vec2 delta_pan, float delta_time) {
    vec3 right = (vec3){ 
        self->base.world_matrix.m[0][0],
        self->base.world_matrix.m[0][1],
        self->base.world_matrix.m[0][2] 
    };

    // Pan X axis in local space
    self->target = vec3_sub(self->target, 
                            vec3_scale(right, delta_pan.x * self->pan_speed.x * delta_time));
    // Pan Y axis in global space
    self->target = vec3_add(self->target,
                            vec3_scale((vec3){0, 1, 0}, 
                                      delta_pan.y * self->pan_speed.y * delta_time));

    // Reset zoom to allow infinite zooming after motion
    float mid_zoom = self->zoom_distance_limit.x + 
                     (self->zoom_distance_limit.y - self->zoom_distance_limit.x) * 0.5f;
    self->zoom_distance = mid_zoom - self->zoom_distance;
    
    vec3 rotation = (vec3){ self->current_rotation.y, self->current_rotation.x, 0 };
    mat3 orient = mat3_rotation(rotation.x, rotation.y, rotation.z);
    vec3 dir = mat3_multiply_vector(orient, (vec3){0.0f, 0.0f, -self->zoom_distance});
    
    self->target = vec3_sub(self->target, dir);
    self->zoom_distance = mid_zoom;
}

void orbit_camera_update(OrbitCamera* self, float delta_time) {
    (void)delta_time;  // Unused in original
    
    vec3 rotation = (vec3){ self->current_rotation.y, self->current_rotation.x, 0 };
    mat3 orient = mat3_rotation(rotation.x, rotation.y, rotation.z);
    vec3 dir = mat3_multiply_vector(orient, (vec3){0.0f, 0.0f, -self->zoom_distance});
    vec3 position = vec3_add(dir, self->target);

    mat4 look_at = mat4_look_at(position, self->target, (vec3){0, 1, 0});
    
#ifndef NO_EXTRAS
    self->base.world_matrix = mat4_fast_inverse(look_at);
#else
    self->base.world_matrix = mat4_inverse(look_at);
#endif
}

void orbit_camera_set_target(OrbitCamera* self, vec3 new_target) {
    self->target = new_target;
}

void orbit_camera_set_zoom(OrbitCamera* self, float zoom) {
    self->zoom_distance = zoom;
}

void orbit_camera_set_rotation(OrbitCamera* self, vec2 rotation) {
    self->current_rotation = rotation;
}

void orbit_camera_print_debug(const OrbitCamera* self) {
    printf("Target: (%.2f, %.2f, %.2f)\n", 
           self->target.x, self->target.y, self->target.z);
    printf("Zoom distance: %.2f\n", self->zoom_distance);
    printf("Rotation: (%.2f, %.2f)\n", 
           self->current_rotation.x, self->current_rotation.y);
}