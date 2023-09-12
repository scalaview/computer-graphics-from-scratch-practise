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
        memset(ctx.point_result, 0, sizeof(i32) * (p1.x - p0.x));
        interpolate(p0.x, p0.y, p1.x, p1.y, ctx.point_result);
        for (i32 i = p0.x; i < p1.x; i++)
        {
            i32 idx = i - p0.x;
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, i), CENTER_TO_ZERO_Y(ctx.height, (*ctx.point_result)[idx]), color.argb);
        }
    }
    else
    {
        if (p0.y > p1.y)
        {
            vec2_swap(&p0, &p1);
        }

        memset(ctx.point_result, 0, sizeof(i32) * (p1.y - p0.y));
        interpolate(p0.y, p0.x, p1.y, p1.x, ctx.point_result);
        for (i32 i = p0.y; i < p1.y; i++)
        {
            i32 idx = i - p0.y;
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, (*ctx.point_result)[idx]), CENTER_TO_ZERO_Y(ctx.height, i), color.argb);
        }
    }
}

void render_frame(canvas canvas)
{
    vec2 p0 = {-200, -100};
    vec2 p1 = {240, 120};
    color color = {.argb = 0x0};
    draw_line(canvas, p0, p1, color);

    p0 = (vec2){-50, -200};
    p1 = (vec2){60, 240};
    draw_line(canvas, p0, p1, color);
}
