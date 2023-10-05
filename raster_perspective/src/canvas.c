#include <string.h>

#include "canvas.h"
#include "logger.h"
#include "rmath.h"

const color backgroud_color = {.argb = 0xffffff};
static const vec3 viewport_vector = {1, 1, 1};
color BLUE = {.b = 0, .g = 0, .r = 255, .alpha = 255};
color RED = {.b = 255, .g = 0, .r = 0, .alpha = 255};
color GREEN = {.b = 0, .g = 255, .r = 0, .alpha = 255};

void vec3_swap(vec3 *p0, vec3 *p1)
{
    vec3 p = *p0;
    p0->x = p1->x;
    p0->y = p1->y;
    p0->p = p1->p;
    p1->x = p.x;
    p1->y = p.y;
    p1->p = p.p;
}

#define INTERPOLATE_HELPER(n) n##_interpolate

#define INTERPOLATE_DEF(t)                                                                               \
    void INTERPOLATE_HELPER(t)(const f64 i0, const f64 d0, const f64 i1, const f64 d1, t(*out_result)[]) \
    {                                                                                                    \
        f64 a = (d1 - d0) / (i1 - i0);                                                                   \
        f64 d = d0;                                                                                      \
        for (i32 i = 0; i < i1 - i0; i++)                                                                \
        {                                                                                                \
            (*out_result)[i] = d;                                                                        \
            d += a;                                                                                      \
        }                                                                                                \
    }

INTERPOLATE_DEF(i32);
INTERPOLATE_DEF(f64);

void draw_line(canvas ctx, vec3 p0, vec3 p1, color color)
{
    if (fabs(p0.x - p1.x) > fabs(p0.y - p1.y))
    {
        if (p0.x > p1.x)
        {
            vec3_swap(&p0, &p1);
        }
        memset(ctx.line_point_result, 0, sizeof(i32) * (p1.x - p0.x));
        i32_interpolate(p0.x, p0.y, p1.x, p1.y, ctx.line_point_result);
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
            vec3_swap(&p0, &p1);
        }

        memset(ctx.line_point_result, 0, sizeof(i32) * (p1.y - p0.y));
        i32_interpolate(p0.y, p0.x, p1.y, p1.x, ctx.line_point_result);
        for (i32 i = p0.y; i < p1.y; i++)
        {
            i32 idx = i - p0.y;
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, (*ctx.line_point_result)[idx]), CENTER_TO_ZERO_Y(ctx.height, i), color.argb);
        }
    }
}

RINLINE vec2 view_port_to_canvas(canvas ctx, f64 x, f64 y)
{
    return (vec2){(x * ctx.width / viewport_vector.w), (y * ctx.height / viewport_vector.h)};
}

RINLINE vec3 project_vertex(canvas ctx, vec3 v)
{
    vec2 v2 = view_port_to_canvas(ctx, v.x * viewport_vector.z / v.z, v.y * viewport_vector.z / v.z);
    return (vec3){v2.x, v2.y, 1};
}

void render_frame(canvas canvas)
{
    vec3 vAf = {-2, -0.5, 5};
    vec3 vBf = {-2, 0.5, 5};
    vec3 vCf = {-1, 0.5, 5};
    vec3 vDf = {-1, -0.5, 5};

    vec3 vAb = {-2, -0.5, 6};
    vec3 vBb = {-2, 0.5, 6};
    vec3 vCb = {-1, 0.5, 6};
    vec3 vDb = {-1, -0.5, 6};

    // front
    draw_line(canvas, project_vertex(canvas, vAf), project_vertex(canvas, vBf), BLUE);
    draw_line(canvas, project_vertex(canvas, vBf), project_vertex(canvas, vCf), BLUE);
    draw_line(canvas, project_vertex(canvas, vCf), project_vertex(canvas, vDf), BLUE);
    draw_line(canvas, project_vertex(canvas, vDf), project_vertex(canvas, vAf), BLUE);

    // back
    draw_line(canvas, project_vertex(canvas, vAb), project_vertex(canvas, vBb), RED);
    draw_line(canvas, project_vertex(canvas, vBb), project_vertex(canvas, vCb), RED);
    draw_line(canvas, project_vertex(canvas, vCb), project_vertex(canvas, vDb), RED);
    draw_line(canvas, project_vertex(canvas, vDb), project_vertex(canvas, vAb), RED);

    // connect the front points to back points
    draw_line(canvas, project_vertex(canvas, vAf), project_vertex(canvas, vAb), GREEN);
    draw_line(canvas, project_vertex(canvas, vBf), project_vertex(canvas, vBb), GREEN);
    draw_line(canvas, project_vertex(canvas, vCf), project_vertex(canvas, vCb), GREEN);
    draw_line(canvas, project_vertex(canvas, vDf), project_vertex(canvas, vDb), GREEN);
}
