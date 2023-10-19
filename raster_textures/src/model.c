#include "model.h"
#include "rmath.h"
#include <stdlib.h>
#include <string.h>

extern const color BLUE;
extern const color RED;
extern const color GREEN;
extern const color YELLOW;
extern const color PURPLE;
extern const color CYAN;
extern const color WHITE;
extern const color BLACK;

mat4x4 identity4x4 = {
    .vec_0 = (vec4){1, 0, 0, 0},
    .vec_1 = (vec4){0, 1, 0, 0},
    .vec_2 = (vec4){0, 0, 1, 0},
    .vec_3 = (vec4){0, 0, 0, 1},
};

vec3 vertices[] = {
    {1, 1, 1},
    {-1, 1, 1},
    {-1, -1, 1},
    {1, -1, 1},
    {1, 1, -1},
    {-1, 1, -1},
    {-1, -1, -1},
    {1, -1, -1}};

model cube = {
    .vertices = &vertices,
    .vertices_count = 8,
    .triangles_count = 12,
    .bounds_center = {0, 0, 0}};

// sphere model
i32 divs = 15;
vec3 sphere_vertices[(15 + 1) * 15];
triangle sphere_triangles[2 * 15 * 15];
model sphere = {.vertices = &sphere_vertices, .triangles = &sphere_triangles, .vertices_count = (15 + 1) * 15, .triangles_count = 2 * 15 * 15};

i32 instances_count = 3;
instance (*instances)[];

mat4x4 make_transform_mm4(instance *instance)
{
    return vec4_mul_mm(vec4_make_translation_matrix(instance->position), vec4_mul_mm(instance->orientation, vec4_make_scaling_matrix(instance->scale)));
}

mat4x4 make_camera_matrix(camera *camera)
{
    return vec4_mul_mm(vec4_transpose(camera->orientation), vec4_make_translation_matrix(vec3_mul_scalar(camera->position, -1)));
}

// ----- Sphere model generator -----
void generate_sphere_model(i32 divs, color color, model *out_model)
{
    vec3(*vertices)[] = out_model->vertices;
    i32 vertices_count = 0;
    triangle(*triangles)[] = out_model->triangles;
    i32 triangles_count = 0;

    f64 delta_angle = 2.0 * M_PI / divs;
    for (i32 d = 0; d < divs + 1; d++)
    {
        f64 y = (2.0 / divs) * (d - divs / 2.0);
        f64 radius = sqrt(1.0 - y * y);
        for (i32 i = 0; i < divs; i++)
        {
            (*vertices)[vertices_count++] = (vec3){.x = radius * cos(i * delta_angle), .y = y, .z = radius * sin(i * delta_angle)};
        }
    }

    for (i32 d = 0; d < divs; d++)
    {
        for (i32 i = 0; i < divs; i++)
        {
            i32 i0 = d * divs + i;
            i32 i1 = (d + 1) * divs + (i + 1) % divs;
            i32 i2 = divs * d + (i + 1) % divs;

            (*triangles)[triangles_count++] = (triangle){.p1 = i0, .p2 = i1, .p3 = i2, .color = color, .normals = {(*vertices)[i0], (*vertices)[i1], (*vertices)[i2]}};
            (*triangles)[triangles_count++] = (triangle){.p1 = i0, .p2 = (i0 + divs), .p3 = i1, .color = color, .normals = {(*vertices)[i0], (*vertices)[(i0 + divs)], (*vertices)[i1]}};
        }
    }
}

void initialize_models()
{
    triangle triangles[] = {
        {0, 1, 2, RED, .normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}}},
        {0, 2, 3, RED, .normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}}},
        {4, 0, 3, GREEN, .normals = {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}}},
        {4, 3, 7, GREEN, .normals = {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}}},
        {5, 4, 7, BLUE, .normals = {{0, 0, -1}, {0, 0, -1}, {0, 0, -1}}},
        {5, 7, 6, BLUE, .normals = {{0, 0, -1}, {0, 0, -1}, {0, 0, -1}}},
        {1, 5, 6, YELLOW, .normals = {{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}}},
        {1, 6, 2, YELLOW, .normals = {{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}}},
        {1, 0, 5, PURPLE, .normals = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}}},
        {5, 0, 4, PURPLE, .normals = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}}},
        {2, 6, 7, CYAN, .normals = {{0, -1, 0}, {0, -1, 0}, {0, -1, 0}}},
        {2, 7, 3, CYAN, .normals = {{0, -1, 0}, {0, -1, 0}, {0, -1, 0}}}};
    cube.triangles_count = 12;
    cube.bounds_radius = sqrt(3);
    cube.triangles = malloc(sizeof(triangle) * cube.triangles_count);
    memcpy(cube.triangles, triangles, sizeof(triangle) * cube.triangles_count);
    cube.transform_and_clip_triangles = malloc(sizeof(triangle) * cube.triangles_count);
    cube.transform_and_clip_new_triangles = malloc(sizeof(triangle) * cube.triangles_count);
    cube.render_scene_vertexs = malloc(sizeof(vec3) * cube.vertices_count);
    cube.render_scene_triangles = malloc(sizeof(triangle) * cube.triangles_count);

    sphere.transform_and_clip_triangles = malloc(sizeof(triangle) * sphere.triangles_count);
    sphere.transform_and_clip_new_triangles = malloc(sizeof(triangle) * sphere.triangles_count);
    sphere.render_scene_vertexs = malloc(sizeof(vec3) * sphere.vertices_count);
    sphere.render_scene_triangles = malloc(sizeof(triangle) * sphere.triangles_count);
    generate_sphere_model(divs, GREEN, &sphere);
}

void free_models()
{
    free(cube.transform_and_clip_triangles);
    free(cube.transform_and_clip_new_triangles);
    free(cube.render_scene_vertexs);
    free(cube.render_scene_triangles);

    free(sphere.transform_and_clip_triangles);
    free(sphere.transform_and_clip_new_triangles);
    free(sphere.render_scene_vertexs);
    free(sphere.render_scene_triangles);
}

void initialize_instances()
{
    instance t_instances[3] = {
        {.model = &cube, .position = {-1.5, 0, 7}, .orientation = identity4x4, .scale = 0.75},
        {.model = &cube, .position = {1.25, 2.5, 7.5}, .orientation = vec4_make_rotation_matrix(195), .scale = 1},
        {.model = &sphere, .position = {1.75, -0.5, 7}, .orientation = identity4x4, .scale = 1.5}};
    instances = malloc(sizeof(instance) * instances_count);
    memcpy(instances, &t_instances, sizeof(instance) * instances_count);
    i32 instances_count = 3;
    for (i16 i = 0; i < instances_count; i++)
    {
        (*instances)[i].transform = make_transform_mm4(&((*instances)[i]));
        (*instances)[i].projected = malloc(sizeof(vec3) * ((*instances)[i].model)->vertices_count);
    }
}

void free_instances()
{
    for (i32 i = 0; i < instances_count; i++)
    {
        free((*instances)[i].projected);
    }
    free(instances);
}