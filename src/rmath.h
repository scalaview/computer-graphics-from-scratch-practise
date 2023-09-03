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