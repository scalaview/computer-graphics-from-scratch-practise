#pragma once
#include "define_types.h"

#define M_PI 3.14159265358979323846

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
            f64 x, w;
        };
        union
        {
            f64 y, h;
        };
        union
        {
            f64 z, p;
        };
    };
} vec3;

typedef union vec4_u
{
    f64 elements[4];
    struct
    {
        union
        {
            f64 x;
        };
        union
        {
            f64 y;
        };
        union
        {
            f64 z;
        };
        union
        {
            f64 w;
        };
    };
} vec4;

typedef union mat4x4
{
    vec4 elements[4];
    struct
    {
        union
        {
            vec4 vec_0;
        };
        union
        {
            vec4 vec_1;
        };
        union
        {
            vec4 vec_2;
        };
        union
        {
            vec4 vec_3;
        };
    };

} mat4x4;

extern mat4x4 identity4x4;