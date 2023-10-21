
#include <stdlib.h>
#include <string.h>

#include "model.h"
#include "rmath.h"
#include "logger.h"

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

i32 instances_count = 2;
instance (*instances)[];

image wood_texture;
image checkerboard_texture;

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

void load_model_resources()
{
    // read images
    if (!read_image_from_file("../assets/crate-texture.jpg", &wood_texture))
    {
        DEBUG("Failed to load image");
        return;
    }

    if (!read_image_from_file("../assets/checkerboard.png", &checkerboard_texture))
    {
        DEBUG("Failed to load image");
        return;
    }
}

void free_model_resources()
{
    free_image(&wood_texture);
    free_image(&checkerboard_texture);
}

void initialize_models()
{
    load_model_resources();

    triangle triangles[] = {
        {0, 1, 2, RED, .normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 0}, {1, 1}}},
        {0, 2, 3, RED, .normals = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 1}, {0, 1}}},
        {4, 0, 3, GREEN, .normals = {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 0}, {1, 1}}},
        {4, 3, 7, GREEN, .normals = {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 1}, {0, 1}}},
        {5, 4, 7, BLUE, .normals = {{0, 0, -1}, {0, 0, -1}, {0, 0, -1}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 0}, {1, 1}}},
        {5, 7, 6, BLUE, .normals = {{0, 0, -1}, {0, 0, -1}, {0, 0, -1}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 1}, {0, 1}}},
        {1, 5, 6, YELLOW, .normals = {{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 0}, {1, 1}}},
        {1, 6, 2, YELLOW, .normals = {{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 1}, {0, 1}}},
        {1, 0, 5, PURPLE, .normals = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 0}, {1, 1}}},
        {5, 0, 4, PURPLE, .normals = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 1}, {1, 1}, {0, 0}}},
        {2, 6, 7, CYAN, .normals = {{0, -1, 0}, {0, -1, 0}, {0, -1, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 0}, {1, 1}}},
        {2, 7, 3, CYAN, .normals = {{0, -1, 0}, {0, -1, 0}, {0, -1, 0}}, .texture = &checkerboard_texture, .uvs = {{0, 0}, {1, 1}, {0, 1}}}};
    cube.triangles_count = 12;
    cube.bounds_radius = sqrt(3);
    cube.triangles = malloc(sizeof(triangle) * cube.triangles_count);
    memcpy(cube.triangles, triangles, sizeof(triangle) * cube.triangles_count);
    cube.transform_and_clip_triangles = malloc(sizeof(triangle) * cube.triangles_count);
    cube.transform_and_clip_new_triangles = malloc(sizeof(triangle) * cube.triangles_count);
    cube.render_scene_vertexs = malloc(sizeof(vec3) * cube.vertices_count);
    cube.render_scene_triangles = malloc(sizeof(triangle) * cube.triangles_count);
}

void free_models()
{
    free(cube.transform_and_clip_triangles);
    free(cube.transform_and_clip_new_triangles);
    free(cube.render_scene_vertexs);
    free(cube.render_scene_triangles);
    free_model_resources();
}

void initialize_instances()
{
    instance t_instances[2] = {
        {.model = &cube, .position = {-1, 0, 3}, .orientation = vec4_make_rotation_matrix(-15), .scale = 1},
        {.model = &cube, .position = {5.5, 5, 20}, .orientation = vec4_make_rotation_matrix(45), .scale = 1}};
    instances = malloc(sizeof(instance) * instances_count);
    memcpy(instances, &t_instances, sizeof(instance) * instances_count);
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