#pragma once

#include "rmath.h"
#include "define_types.h"
#include "color.h"

typedef struct sphere
{
    vec3 center;
    i32 radius;
    color color;
} sphere;

typedef struct canvas
{
    void (*put_pixel)(i32 x, i32 y, u32 color);
    i32 width;
    i32 height;
    sphere (*spheres)[];
    i32 sphere_size;
} canvas;

void render_frame(canvas canvas);
extern const color backgroud_color;
#define CENTER_TO_ZERO_X(width, x) (width / 2 + x)
#define CENTER_TO_ZERO_Y(height, x) (height / 2 - y - 1)