#pragma once
#include "define_types.h"

typedef union vec2_u
{
    f32 elements[2];
    struct
    {
        union
        {
            f32 x, w;
        };
        union
        {
            f32 y, h;
        };
    };
} vec2;

typedef union vec3_u
{
    f32 elements[3];
    struct
    {
        union
        {
            f32 x, w, r;
        };
        union
        {
            f32 y, h, g;
        };
        union
        {
            f32 z, b;
        };
    };
} vec3;

typedef vec3 rgb;