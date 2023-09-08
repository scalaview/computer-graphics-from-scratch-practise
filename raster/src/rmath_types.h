#pragma once
#include "define_types.h"

typedef union vec2_u
{
    f64 elements[2];
    struct
    {
        union
        {
            f64 x, w;
        };
        union
        {
            f64 y, h;
        };
    };
} vec2;

typedef union vec3_u
{
    f64 elements[3];
    struct
    {
        union
        {
            f64 x, w, r;
        };
        union
        {
            f64 y, h, g;
        };
        union
        {
            f64 z, b;
        };
    };
} vec3;

typedef vec3 rgb;