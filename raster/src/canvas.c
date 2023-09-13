#include <string.h>

#include "canvas.h"
#include "logger.h"
#include "rmath.h"

const color backgroud_color = {.argb = 0xffffff};

void vec2_swap(vec2 *p0, vec2 *p1)
{
    vec2 p = *p0;
    p0->x = p1->x;
    p0->y = p1->y;
    p1->x = p.x;
    p1->y = p.y;
}

void interpolate(const f64 i0, const f64 d0, const f64 i1, const f64 d1, i32 (*out_result)[])
{
    f64 a = (d1 - d0) / (i1 - i0);
    f64 d = d0;
    for (i32 i = 0; i < i1 - i0; i++)
    {
        (*out_result)[i] = d;
        d += a;
    }
}

void draw_line(canvas ctx, vec2 p0, vec2 p1, color color)
{
    if (fabs(p0.x - p1.x) > fabs(p0.y - p1.y))
    {
        if (p0.x > p1.x)
        {
            vec2_swap(&p0, &p1);
        }
        memset(ctx.line_point_result, 0, sizeof(i32) * (p1.x - p0.x));
        interpolate(p0.x, p0.y, p1.x, p1.y, ctx.line_point_result);
        for (i32 i = p0.x; i < p1.x; i++)
        {
            i32 idx = i - p0.x;
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, i), CENTER_TO_ZERO_Y(ctx.height, (*ctx.line_point_result)[idx]), color.argb);
        }
    }
    else
    {
        if (p0.y > p1.y)
        {
            vec2_swap(&p0, &p1);
        }

        memset(ctx.line_point_result, 0, sizeof(i32) * (p1.y - p0.y));
        interpolate(p0.y, p0.x, p1.y, p1.x, ctx.line_point_result);
        for (i32 i = p0.y; i < p1.y; i++)
        {
            i32 idx = i - p0.y;
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, (*ctx.line_point_result)[idx]), CENTER_TO_ZERO_Y(ctx.height, i), color.argb);
        }
    }
}

void draw_filled_triangle(canvas ctx, vec2 p0, vec2 p1, vec2 p2, color color)
{
    if (p0.y > p1.y)
    {
        vec2_swap(&p0, &p1);
    }
    if (p0.y > p2.y)
    {
        vec2_swap(&p0, &p2);
    }
    if (p1.y > p2.y)
    {
        vec2_swap(&p1, &p2);
    }

    interpolate(p0.y, p0.x, p1.y, p1.x, ctx.triangle_point_result01);
    interpolate(p1.y, p1.x, p2.y, p2.x, ctx.triangle_point_result12);
    interpolate(p0.y, p0.x, p2.y, p2.x, ctx.triangle_point_result02);

    i32 result_01_len = p1.y - p0.y;
    i32 result_12_len = p2.y - p1.y;
    i32 *x01_tail_ptr = ((i32 *)ctx.triangle_point_result01) + (result_01_len - 1);
    for (i32 i = 0; i < result_12_len; i++, x01_tail_ptr++)
    {
        *x01_tail_ptr = (*ctx.triangle_point_result12)[i];
    }
    i32 middle = floor((result_01_len + result_12_len - 1) / 2);
    i32(*x_left)[] = 0;
    i32(*x_right)[] = 0;

    if ((*ctx.triangle_point_result02)[middle] < (*ctx.triangle_point_result01)[middle])
    {
        x_left = ctx.triangle_point_result02;
        x_right = ctx.triangle_point_result01;
    }
    else
    {
        x_left = ctx.triangle_point_result01;
        x_right = ctx.triangle_point_result02;
    }

    for (i32 y = p0.y; y < p2.y; y++)
    {
        for (i32 x = (*x_left)[(i32)(y - p0.y)]; x < (*x_right)[(i32)(y - p0.y)]; x++)
        {
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, x), CENTER_TO_ZERO_Y(ctx.height, y), color.argb);
        }
    }
}

void draw_wire_frame_triangle(canvas ctx, vec2 p0, vec2 p1, vec2 p2, color color)
{
    draw_line(ctx, p0, p1, color);
    draw_line(ctx, p1, p2, color);
    draw_line(ctx, p2, p0, color);
}

void render_frame(canvas canvas)
{
    vec2 p0 = {-200, -250};
    vec2 p1 = {200, 50};
    vec2 p2 = {20, 250};
    color green_color = {.r = 0, .g = 255, .b = 0};
    color black_color = {.argb = 0x0};
    draw_filled_triangle(canvas, p0, p1, p2, green_color);
    draw_wire_frame_triangle(canvas, p0, p1, p2, black_color);
}
