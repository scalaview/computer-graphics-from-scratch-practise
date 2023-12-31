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
    f64 (*depth_buffer)[];
} canvas;

void render_frame(canvas canvas);
extern const color backgroud_color;
#define CENTER_TO_ZERO_X(width, x) (width / 2 + x)
#define CENTER_TO_ZERO_Y(height, y) (height / 2 - y - 1)

void prepare_memory_buffer(canvas canvas_ctx);
void free_memory_buffer();
