#pragma once
#include "define_types.h"
#include "color.h"

typedef struct triangle
{
    u32 p1, p2, p3;
    color color;
} triangle;

typedef struct model
{
    vec3 (*vertices)[];
    i32 vertices_count;
    triangle (*triangles)[];
    i32 triangles_count;
    vec3 bounds_center;
    f64 bounds_radius;
} model;

typedef struct instance
{
    model *model;
    vec3 position;
    mat4x4 orientation;
    f64 scale;
    mat4x4 transform;
} instance;

mat4x4 make_transform_mm4(instance *instance);

typedef struct plane
{
    vec3 normal;
    f64 distance;
} plane;
typedef struct camera
{
    vec3 position;
    mat4x4 orientation;
    plane (*clipping_planes)[];
    i32 clipping_plane_count;
} camera;
mat4x4 make_camera_matrix(camera *camera);
