#pragma once

#include "rmath.h"
#include "define_types.h"
#include "color.h"

typedef struct canvas
{
    void (*put_pixel)(i32 x, i32 y, u32 color);
    i32 width;
    i32 height;
    i32 (*line_point_result)[];
    i32 (*triangle_point_result01)[];
    i32 (*triangle_point_result12)[];
    i32 (*triangle_point_result02)[];
} canvas;

void render_frame(canvas canvas);
extern const color backgroud_color;
#define CENTER_TO_ZERO_X(width, x) (width / 2 + x)
#define CENTER_TO_ZERO_Y(height, y) (height / 2 - y - 1)