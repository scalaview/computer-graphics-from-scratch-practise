#pragma once

#include <math.h>
#include "rmath_types.h"

RINLINE f32 vec3_dot(vec3 vector_0, vec3 vector_1)
{
    f32 p = 0;
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

RINLINE f32 vec3_length_squared(vec3 vector)
{
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

RINLINE f32 vec3_length(vec3 vector)
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

RINLINE void vec3_normalize(vec3 *vector)
{
    const f32 length = vec3_length(*vector);
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
}

RINLINE vec3 vec3_normalized(vec3 vector)
{
    vec3_normalize(&vector);
    return vector;
}