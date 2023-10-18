#pragma once

#include <math.h>
#include "rmath_types.h"

RINLINE vec3 vec2_to_vec3(vec2 v)
{
    return (vec3){.x = v.x, .y = v.y, .z = 1};
}

RINLINE f64 vec3_dot(vec3 vector_0, vec3 vector_1)
{
    f64 p = 0;
    p += vector_0.x * vector_1.x;
    p += vector_0.y * vector_1.y;
    p += vector_0.z * vector_1.z;
    return p;
}

RINLINE vec3 vec3_sub(vec3 vector_0, vec3 vector_1)
{
    return (vec3){
        vector_0.x - vector_1.x,
        vector_0.y - vector_1.y,
        vector_0.z - vector_1.z};
}

RINLINE f64 vec3_length_squared(vec3 vector)
{
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

RINLINE f64 vec3_length(vec3 vector)
{
    return sqrt(vec3_length_squared(vector));
}

RINLINE vec3 vec3_add(vec3 vector_0, vec3 vector_1)
{
    return (vec3){
        vector_0.x + vector_1.x,
        vector_0.y + vector_1.y,
        vector_0.z + vector_1.z};
}

RINLINE vec3 vec3_mul_scalar(vec3 vector_0, f64 scalar)
{
    return (vec3){
        vector_0.x * scalar,
        vector_0.y * scalar,
        vector_0.z * scalar};
}

RINLINE void vec3_normalize(vec3 *vector)
{
    const f64 length = vec3_length(*vector);
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
}

RINLINE vec3 vec3_normalized(vec3 vector)
{
    vec3_normalize(&vector);
    return vector;
}

// Multiplies a matrix and a vector.
RINLINE vec3 vec3_mul_mv(vec3 vector_mat[], vec3 vector_0)
{
    return (vec3){
        vec3_dot(vector_0, (vec3){vector_mat[0].x, vector_mat[0].y, vector_mat[0].z}),
        vec3_dot(vector_0, (vec3){vector_mat[1].x, vector_mat[1].y, vector_mat[1].z}),
        vec3_dot(vector_0, (vec3){vector_mat[2].x, vector_mat[2].y, vector_mat[2].z})};
}

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

RINLINE vec3 vec3_cross(vec3 vector_0, vec3 vector_1)
{
    return (vec3){
        vector_0.y * vector_1.z - vector_0.z * vector_1.y,
        vector_0.z * vector_1.x - vector_0.x * vector_1.z,
        vector_0.x * vector_1.y - vector_0.y * vector_1.x};
}

RINLINE vec4 vec3_to_vec4(vec3 v)
{
    return (vec4){.x = v.x, .y = v.y, .z = v.z, .w = 1};
}

// vec4 dot
RINLINE f64 vec4_dot(vec4 vector_0, vec4 vector_1)
{
    f64 p = 0;
    p += vector_0.x * vector_1.x;
    p += vector_0.y * vector_1.y;
    p += vector_0.z * vector_1.z;
    p += vector_0.w * vector_1.w;
    return p;
}

// transform matrix for vec4
RINLINE mat4x4 vec4_make_translation_matrix(vec3 vector)
{
    return (mat4x4){
        .vec_0 = {1, 0, 0, vector.x},
        .vec_1 = {0, 1, 0, vector.y},
        .vec_2 = {0, 0, 1, vector.z},
        .vec_3 = {0, 0, 0, 1}};
}

// Makes a transform matrix for a rotation around the OY axis.
RINLINE mat4x4 vec4_make_rotation_matrix(f64 radians)
{
    f64 cos_radians = cos(radians * M_PI / 180);
    f64 sin_radians = sin(radians * M_PI / 180);
    return (mat4x4){
        .vec_0 = {cos_radians, 0, -sin_radians, 0},
        .vec_1 = {0, 1, 0, 0},
        .vec_2 = {sin_radians, 0, cos_radians, 0},
        .vec_3 = {0, 0, 0, 1}};
}

// Makes a transform matrix for a scaling.
RINLINE mat4x4 vec4_make_scaling_matrix(f64 scaling)
{
    return (mat4x4){
        .vec_0 = {scaling, 0, 0, 0},
        .vec_1 = {0, scaling, 0, 0},
        .vec_2 = {0, 0, scaling, 0},
        .vec_3 = {0, 0, 0, 1}};
}

// Multiplies a 4x4 matrix and a 4D vector.
RINLINE vec4 vec4_mul_mv(mat4x4 vector_mat, vec4 vector_0)
{
    return (vec4){
        vec4_dot(vector_0, (vec4){vector_mat.vec_0.x, vector_mat.vec_0.y, vector_mat.vec_0.z, vector_mat.vec_0.w}),
        vec4_dot(vector_0, (vec4){vector_mat.vec_1.x, vector_mat.vec_1.y, vector_mat.vec_1.z, vector_mat.vec_1.w}),
        vec4_dot(vector_0, (vec4){vector_mat.vec_2.x, vector_mat.vec_2.y, vector_mat.vec_2.z, vector_mat.vec_2.w}),
        vec4_dot(vector_0, (vec4){vector_mat.vec_3.x, vector_mat.vec_3.y, vector_mat.vec_3.z, vector_mat.vec_3.w})};
}

// Multiplies two 4x4 matrices. This is the same as matrix multiplication.
RINLINE mat4x4 vec4_mul_mm(mat4x4 vector_mat_1, mat4x4 vector_mat_2)
{
    mat4x4 result = {};
    for (i16 i = 0; i < 4; i++)
    {
        vec4 *v4 = &result.elements[i];
        for (i16 j = 0; j < 4; j++)
        {
            v4->elements[j] = vec4_dot(vector_mat_1.elements[i], (vec4){vector_mat_2.elements[0].elements[j], vector_mat_2.elements[1].elements[j], vector_mat_2.elements[2].elements[j], vector_mat_2.elements[3].elements[j]});
        }
    }
    return result;
}

// Transposes a 4x4 matrix.
RINLINE mat4x4 vec4_transpose(mat4x4 vector_mat)
{
    return (mat4x4){
        .vec_0 = {vector_mat.vec_0.x, vector_mat.vec_1.x, vector_mat.vec_2.x, vector_mat.vec_3.x},
        .vec_1 = {vector_mat.vec_0.y, vector_mat.vec_1.y, vector_mat.vec_2.y, vector_mat.vec_3.y},
        .vec_2 = {vector_mat.vec_0.z, vector_mat.vec_1.z, vector_mat.vec_2.z, vector_mat.vec_3.z},
        .vec_3 = {vector_mat.vec_0.w, vector_mat.vec_1.w, vector_mat.vec_2.w, vector_mat.vec_3.w}};
}

RINLINE vec3 vec4_to_vec3(vec4 vector)
{
    return (vec3){vector.x, vector.y, vector.z};
}