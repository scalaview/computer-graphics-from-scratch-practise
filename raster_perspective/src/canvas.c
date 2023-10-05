#include <string.h>

#include "canvas.h"
#include "logger.h"
#include "rmath.h"

const color backgroud_color = {.argb = 0xffffff};
static const vec3 viewport_vector = {1, 1, 1};
const color BLUE = {.b = 0, .g = 0, .r = 255, .alpha = 255};
const color RED = {.b = 255, .g = 0, .r = 0, .alpha = 255};
const color GREEN = {.b = 0, .g = 255, .r = 0, .alpha = 255};
const color YELLOW = {.b = 255, .g = 255, .r = 0, .alpha = 255};
const color PURPLE = {.b = 255, .g = 0, .r = 255, .alpha = 255};
const color CYAN = {.b = 0, .g = 255, .r = 255, .alpha = 255};

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

void draw_wire_frame_triangle(canvas ctx, vec3 p0, vec3 p1, vec3 p2, color color)
{
    draw_line(ctx, p0, p1, color);
    draw_line(ctx, p1, p2, color);
    draw_line(ctx, p2, p0, color);
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

void render_triangle(canvas ctx, triangle triangle, vec3 (*vertices)[])
{
    draw_wire_frame_triangle(ctx, (*vertices)[triangle.p1], (*vertices)[triangle.p2], (*vertices)[triangle.p3], triangle.color);
}

void render_object(canvas ctx, vec3 (*vertices)[], triangle (*triangles)[])
{
    for (i32 i = 0; i < ctx.projected_size; i++)
    {
        (*ctx.projected)[i] = project_vertex(ctx, (*vertices)[i]);
    }

    for (i32 i = 0; i < ctx.triangle_size; i++)
    {
        render_triangle(ctx, (*triangles)[i], ctx.projected);
    }
}

void render_frame(canvas canvas)
{
    vec3 vertices[] = {
        {1, 1, 1},
        {-1, 1, 1},
        {-1, -1, 1},
        {1, -1, 1},
        {1, 1, -1},
        {-1, 1, -1},
        {-1, -1, -1},
        {1, -1, -1}};

    triangle triangles[] = {
        {0, 1, 2, RED},
        {0, 2, 3, RED},
        {4, 0, 3, GREEN},
        {4, 3, 7, GREEN},
        {5, 4, 7, BLUE},
        {5, 7, 6, BLUE},
        {1, 5, 6, YELLOW},
        {1, 6, 2, YELLOW},
        {4, 5, 1, PURPLE},
        {4, 1, 0, PURPLE},
        {2, 6, 7, CYAN},
        {2, 7, 3, CYAN}};

    // translate {-1.5, 0, 7}
    for (i32 i = 0; i < canvas.projected_size; i++)
    {
        vertices[i].x -= 1.5;
        vertices[i].z += 7;
    }

    render_object(canvas, &vertices, &triangles);
}
