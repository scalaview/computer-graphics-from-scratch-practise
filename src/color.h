#pragma once
#include "define_types.h"
#include "rmath.h"

typedef union color
{

    u32 argb : 32;
    struct
    {
        u8 b : 8;
        u8 g : 8;
        u8 r : 8;
        u8 alpha : 8;
    } ATTR_PACKED;
} color;

RINLINE color color_mul(color col, vec3 scalar)
{

    return (color){.alpha = col.alpha,
                   .r = MIN(col.r * scalar.r, 255),
                   .g = MIN(col.g * scalar.g, 255),
                   .b = MIN(col.b * scalar.b, 255)};
}