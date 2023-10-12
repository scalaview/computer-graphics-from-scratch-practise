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
    triangle (*triangles)[];
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

typedef struct camera
{
    vec3 position;
    mat4x4 orientation;
} camera;
mat4x4 make_camera_matrix(camera *camera);