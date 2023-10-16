#include <string.h>

#include "canvas.h"
#include "model.h"
#include "logger.h"

const color backgroud_color = {.argb = 0xffffff};
static const vec3 viewport_vector = {1, 1, 1};
const color BLUE = {.r = 0, .g = 0, .b = 255, .alpha = 255};
const color RED = {.r = 255, .g = 0, .b = 0, .alpha = 255};
const color GREEN = {.r = 0, .g = 255, .b = 0, .alpha = 255};
const color YELLOW = {.r = 255, .g = 255, .b = 0, .alpha = 255};
const color PURPLE = {.r = 255, .g = 0, .b = 255, .alpha = 255};
const color CYAN = {.r = 0, .g = 255, .b = 255, .alpha = 255};
#define STA_BUF_SIZE 64
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
        for (i32 i = 0; i <= i1 - i0; i++)                                                               \
        {                                                                                                \
            (*out_result)[i] = d;                                                                        \
            d += a;                                                                                      \
        }                                                                                                \
    }

INTERPOLATE_DEF(i32);
INTERPOLATE_DEF(f64);

RINLINE vec2 view_port_to_canvas(canvas ctx, f64 x, f64 y)
{
    return (vec2){round(x * ctx.width / viewport_vector.w), round(y * ctx.height / viewport_vector.h)};
}

#define PROJECT_VERTEX_HELPER(n) n##_project_vertex
#define PROJECT_VERTEX_DEF(t)                                                                             \
    RINLINE vec3 PROJECT_VERTEX_HELPER(t)(canvas ctx, t v)                                                \
    {                                                                                                     \
        vec2 v2 = view_port_to_canvas(ctx, v.x * viewport_vector.z / v.z, v.y * viewport_vector.z / v.z); \
        return (vec3){v2.x, v2.y, 1};                                                                     \
    }
PROJECT_VERTEX_DEF(vec3);
PROJECT_VERTEX_DEF(vec4);

void draw_line(canvas ctx, vec3 p0, vec3 p1, color color)
{
    if (fabs(p0.x - p1.x) > fabs(p0.y - p1.y))
    {
        if (p0.x > p1.x)
        {
            vec3_swap(&p0, &p1);
        }
        memset(ctx.line_point_result, 0, sizeof(i32) * (p1.x - p0.x + 1));
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

        memset(ctx.line_point_result, 0, sizeof(i32) * (p1.y - p0.y + 1));
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

void render_triangle(canvas ctx, triangle triangle, vec3 (*vertices)[])
{
    draw_wire_frame_triangle(ctx, (*vertices)[triangle.p1], (*vertices)[triangle.p2], (*vertices)[triangle.p3], triangle.color);
}

void render_model(canvas ctx, model *model)
{
    memset(ctx.projected, 0, sizeof(vec3) * ctx.projected_size);
    for (i32 i = 0; i < ctx.projected_size; i++)
    {
        vec3 vertice = (*(model->vertices))[i];
        (*ctx.projected)[i] = vec4_project_vertex(ctx, (vec4){vertice.x, vertice.y, vertice.z, 1});
    }

    for (i32 i = 0; i < ctx.triangle_size; i++)
    {
        render_triangle(ctx, (*(model->triangles))[i], ctx.projected);
    }
}

Bool clip_triangle(canvas ctx, triangle *origin_triangle, plane *clipping_plane, vec3 (*vertices)[], i32 vertices_count, triangle *clipped_triangle)
{
    vec3 v1 = (*vertices)[origin_triangle->p1];
    vec3 v2 = (*vertices)[origin_triangle->p2];
    vec3 v3 = (*vertices)[origin_triangle->p3];

    i16 in1 = (vec3_dot(clipping_plane->normal, v1) + clipping_plane->distance) > 0 ? 1 : 0;
    i16 in2 = (vec3_dot(clipping_plane->normal, v2) + clipping_plane->distance) > 0 ? 1 : 0;
    i16 in3 = (vec3_dot(clipping_plane->normal, v3) + clipping_plane->distance) > 0 ? 1 : 0;

    i16 in_count = in1 + in2 + in3;
    if (in_count == 0)
    {
        // Nothing to do - the triangle is fully clipped out.
        return False;
    }
    else if (in_count == 3)
    {
        // The triangle is fully in front of the plane.
        *clipped_triangle = *origin_triangle;
        return True;
    }
    else if (in_count == 1)
    {
        // The triangle has one vertex in. Output is one clipped triangle.
        return False;
    }
    else if (in_count == 2)
    {
        // The triangle has two vertices in. Output is two clipped triangles.
        return False;
    }
    else
    {
        return False;
    }
}

Bool transform_and_clip(canvas ctx, plane (*clipping_planes)[], i32 clipping_plane_count, model *origin_model, f64 scale, mat4x4 transform, model *clipped)
{
    vec4 center_w = vec4_mul_mv(transform, (vec4){origin_model->bounds_center.x, origin_model->bounds_center.y, origin_model->bounds_center.z, 1});
    vec3 center = vec4_to_vec3(center_w);
    f64 radius = origin_model->bounds_radius * scale;
    for (i32 i = 0; i < clipping_plane_count; i++)
    {
        plane plane = (*clipping_planes)[i];
        f64 distance = vec3_dot(plane.normal, center) + plane.distance;
        if (distance < -radius)
            return False;
    }

    // apply vertices transform
    for (i32 i = 0; i < origin_model->vertices_count; i++)
    {
        vec3 vertice = (*origin_model->vertices)[i];
        vec4 vertice_transformed = vec4_mul_mv(transform, (vec4){vertice.x, vertice.y, vertice.z, 1});
        (*clipped->vertices)[i] = vec4_to_vec3(vertice_transformed);
    }

    triangle triangles[STA_BUF_SIZE];
    memcpy(triangles, origin_model->triangles, sizeof(triangle) * origin_model->triangles_count);
    i32 triangles_count = origin_model->triangles_count;
    for (i32 i = 0; i < clipping_plane_count; i++)
    {
        triangle new_triangles[STA_BUF_SIZE];
        i32 idx = 0;
        for (i32 j = 0; j < origin_model->triangles_count; j++)
        {
            triangle triangle = (*origin_model->triangles)[j];
            if (clip_triangle(ctx, &(*origin_model->triangles)[j], &(*clipping_planes)[i], clipped->vertices, clipped->vertices_count, &(new_triangles[idx])))
            {
                idx++;
            }
        }
        if (idx > 0)
        {
            memcpy(&triangles, &new_triangles, sizeof(triangle) * idx);
            triangles_count = idx;
        }
    }
    memcpy(&(*clipped->triangles), &triangles, sizeof(triangle) * triangles_count);
    clipped->triangles_count = triangles_count;
    clipped->bounds_center = center;
    clipped->bounds_radius = radius;
    clipped->vertices_count = origin_model->vertices_count;
    return True;
}

void render_scene(canvas ctx, camera *camera, instance (*instances)[], i32 instance_size)
{
    mat4x4 m_camera = make_camera_matrix(camera);
    for (i32 i = 0; i < instance_size; i++)
    {
        instance instance = (*instances)[i];
        mat4x4 transform = vec4_mul_mm(m_camera, instance.transform);

        vec3 vertices[8];
        triangle triangles[12];
        model clipped = {.vertices = &vertices, .triangles = &triangles};
        if (transform_and_clip(ctx, camera->clipping_planes, camera->clipping_plane_count, instance.model, instance.scale, transform, &clipped))
            render_model(ctx, &clipped);
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

    model cube = {
        &vertices,
        8,
        &triangles,
        12,
        {0, 0, 0},
        sqrt(3)};

    instance instances[] = {
        {&cube, {-1.5, 0, 7}, identity4x4, 0.75},
        {&cube, {1.25, 2, 7.5}, vec4_make_rotation_matrix(195), 1}};
    for (i16 i = 0; i < 2; i++)
    {
        instances[i].transform = make_transform_mm4(&instances[i]);
    }

    f64 sqrt2 = 1 / sqrt(2);
    plane clipping_planes[] = {
        {(vec3){0, 0, 1}, -1},         // Near
        {(vec3){sqrt2, 0, sqrt2}, 0},  // Left
        {(vec3){-sqrt2, 0, sqrt2}, 0}, // Right
        {(vec3){0, -sqrt2, sqrt2}, 0}, // Top
        {(vec3){0, sqrt2, sqrt2}, 0},  // Bottom
    };
    camera camera = {{-3, 1, 2}, vec4_make_rotation_matrix(-30), &clipping_planes, 5};
    render_scene(canvas, &camera, &instances, 2);
}
