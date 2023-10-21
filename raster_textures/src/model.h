#pragma once
#include "define_types.h"
#include "color.h"
#include "image.h"

typedef struct triangle
{
    u32 p1, p2, p3;
    color color;
    vec3 normals[3];
    image *texture;
    vec2 uvs[3]; // texture points
} triangle;

typedef struct model
{
    vec3 (*vertices)[];
    i32 vertices_count;
    triangle (*triangles)[];
    i32 triangles_count;
    vec3 bounds_center;
    f64 bounds_radius;

    // buffers
    triangle (*transform_and_clip_triangles)[];
    triangle (*transform_and_clip_new_triangles)[];

    vec3 (*render_scene_vertexs)[];
    triangle (*render_scene_triangles)[];
} model;

typedef struct instance
{
    model *model;
    vec3 position;
    mat4x4 orientation;
    f64 scale;
    mat4x4 transform;
    vec3 (*projected)[];
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

typedef enum light_type
{
    LIGHT_AMBIENT = 0,
    LIGHT_POINT = 1,
    LIGHT_DIRECTIONAL = 2
} light_type;
typedef struct light
{
    light_type type;
    f32 intensity;
    union
    {
        vec3 position, direction;
    };

} light;

typedef enum
{
    SM_FLAT,
    SM_GOURAUD,
    SM_PHONG
} shading_model_enum;

typedef struct shading_model
{
    shading_model_enum type;
} shading_model;

typedef enum lighting_model
{
    LM_DIFFUSE = 0x1,
    LM_SPECULAR = 0x10
} lighting_model_enum;

void initialize_models();
void free_models();
void initialize_instances();
void free_instances();
