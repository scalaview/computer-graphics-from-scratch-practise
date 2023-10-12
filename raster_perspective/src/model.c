#include "model.h"

mat4x4 identity4x4 = {
    .vec_0 = (vec4){1, 0, 0, 0},
    .vec_1 = (vec4){0, 1, 0, 0},
    .vec_2 = (vec4){0, 0, 1, 0},
    .vec_3 = (vec4){0, 0, 0, 1},
};

mat4x4 make_transform_mm4(instance *instance)
{
    return vec4_mul_mm(vec4_make_translation_matrix(instance->position), vec4_mul_mm(instance->orientation, vec4_make_scaling_matrix(instance->scale)));
}

mat4x4 make_camera_matrix(camera *camera)
{
    return vec4_mul_mm(vec4_transpose(camera->orientation), vec4_make_translation_matrix(vec3_mul_scalar(camera->position, -1)));
}