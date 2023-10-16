#include <string.h>

#include "canvas.h"
#include "model.h"
#include "logger.h"
#include <stdlib.h>

const color backgroud_color = {.argb = 0xffffff};
static const vec3 viewport_vector = {1, 1, 1};
const color BLUE = {.r = 0, .g = 0, .b = 255, .alpha = 255};
const color RED = {.r = 255, .g = 0, .b = 0, .alpha = 255};
const color GREEN = {.r = 0, .g = 255, .b = 0, .alpha = 255};
const color YELLOW = {.r = 255, .g = 255, .b = 0, .alpha = 255};
const color PURPLE = {.r = 255, .g = 0, .b = 255, .alpha = 255};
const color CYAN = {.r = 0, .g = 255, .b = 255, .alpha = 255};
const color WHITE = {.r = 255, .g = 255, .b = 255, .alpha = 255};
#define STA_BUF_SIZE 64

f64 (*bufv01)[];
f64 (*bufv12)[];
f64 (*bufv02)[];

f64 (*x02)[];
f64 (*x012)[];
f64 (*iz02)[];
f64 (*iz012)[];
f64 (*zscan)[];

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
        if (i0 == i1)                                                                                    \
        {                                                                                                \
            (*out_result)[0] = d0;                                                                       \
            return;                                                                                      \
        }                                                                                                \
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

RINLINE vec2 view_port_to_canvas(i32 width, i32 height, f64 x, f64 y)
{
    return (vec2){round(x * width / viewport_vector.w), round(y * height / viewport_vector.h)};
}

#define PROJECT_VERTEX_HELPER(n) n##_project_vertex
#define PROJECT_VERTEX_DEF(t)                                                                                               \
    RINLINE vec3 PROJECT_VERTEX_HELPER(t)(canvas ctx, t v)                                                                  \
    {                                                                                                                       \
        vec2 v2 = view_port_to_canvas(ctx.width, ctx.height, v.x * viewport_vector.z / v.z, v.y * viewport_vector.z / v.z); \
        return (vec3){v2.x, v2.y, 1};                                                                                       \
    }
PROJECT_VERTEX_DEF(vec3);
PROJECT_VERTEX_DEF(vec4);

void shuffle(triangle (*triangles)[], i32 count)
{
    for (i32 i = count - 1; i < 0; i--)
    {
        i32 idx = rand() % (i + 1);
        triangle t = (*triangles)[idx];
        (*triangles)[idx] = (*triangles)[i];
        (*triangles)[i] = t;
    }
}

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

void sorted_vertex_indexes(i32 (*vertex_indexes)[], vec3 (*projected)[], i32 (*out_indexes)[])
{
    i32 indexes[] = {0, 1, 2};

    if ((*projected)[(*vertex_indexes)[indexes[0]]].y > (*projected)[(*vertex_indexes)[indexes[1]]].y)
    {
        i32 swap = indexes[0];
        indexes[0] = indexes[1];
        indexes[1] = swap;
    }

    if ((*projected)[(*vertex_indexes)[indexes[0]]].y > (*projected)[(*vertex_indexes)[indexes[2]]].y)
    {
        i32 swap = indexes[0];
        indexes[0] = indexes[2];
        indexes[2] = swap;
    }

    if ((*projected)[(*vertex_indexes)[indexes[1]]].y > (*projected)[(*vertex_indexes)[indexes[2]]].y)
    {
        i32 swap = indexes[1];
        indexes[1] = indexes[2];
        indexes[2] = swap;
    }

    memcpy(out_indexes, indexes, sizeof(i32) * 3);
}

RINLINE vec3 compute_triangle_normal(vec3 p0, vec3 p1, vec3 p2)
{
    vec3 v0v1 = vec3_add(p1, vec3_mul_scalar(p0, -1));
    vec3 v0v2 = vec3_add(p2, vec3_mul_scalar(p0, -1));
    return vec3_cross(v0v1, v0v2);
}

Bool cull_back_faces(vec3 *vertex_to_camera, vec3 *normal)
{
    return vec3_dot(*normal, *vertex_to_camera) > 0;
}

void edge_interpolate(f64 y0, f64 v0, f64 y1, f64 v1, f64 y2, f64 v2, f64 (*out_v02)[], f64 (*out_v012)[])
{
    f64_interpolate(y0, v0, y1, v1, bufv01);

    f64_interpolate(y1, v1, y2, v2, bufv12);

    f64_interpolate(y0, v0, y2, v2, bufv02);

    i32 v01_len = y1 - y0 + 1;
    i32 v12_len = y2 - y1 + 1;
    i32 v02_len = y2 - y0 + 1;

    f64 *bufv01_tail = ((f64 *)bufv01) + MAX(v01_len - 1, 0);
    for (i32 i = 0; i < v12_len; i++, bufv01_tail++)
    {
        *bufv01_tail = (*bufv12)[i];
    }
    memcpy(out_v02, bufv02, sizeof(f64) * v02_len);
    memcpy(out_v012, bufv01, sizeof(f64) * MAX((v01_len + v12_len - 1), 0));
}

Bool update_depth_buffer_if_closer(canvas ctx, i32 x, i32 y, f64 inv_z)
{
    i32 p_x = CENTER_TO_ZERO_X(ctx.width, x);
    i32 p_y = CENTER_TO_ZERO_Y(ctx.height, y);
    if (p_x < 0 || p_x >= ctx.width || p_y < 0 || p_y >= ctx.height)
        return False;

    i32 offset = p_x + p_y * ctx.width;
    if ((*ctx.depth_buffer)[offset] == 0 || (*ctx.depth_buffer)[offset] < inv_z)
    {
        (*ctx.depth_buffer)[offset] = inv_z;
        return True;
    }
    return False;
}

void draw_filled_triangle(canvas ctx, triangle triangle, vec3 (*vertices)[], vec3 (*projected)[])
{
    vec3 normal = compute_triangle_normal((*vertices)[triangle.p1], (*vertices)[triangle.p2], (*vertices)[triangle.p3]);
    vec3 vertex_to_camera = vec3_mul_scalar((*vertices)[triangle.p1], -1);
    if (!cull_back_faces(&vertex_to_camera, &normal))
    {
        return;
    }
    i32 indexes[] = {0, 1, 2};
    i32 vertex_indexes[] = {triangle.p1, triangle.p2, triangle.p3};
    sorted_vertex_indexes(&vertex_indexes, projected, &indexes);
    i32 i0 = indexes[0];
    i32 i1 = indexes[1];
    i32 i2 = indexes[2];

    vec3 v0 = (*vertices)[vertex_indexes[i0]];
    vec3 v1 = (*vertices)[vertex_indexes[i1]];
    vec3 v2 = (*vertices)[vertex_indexes[i2]];

    // calculate 1/Z
    vec3 p0 = (*projected)[vertex_indexes[i0]];
    vec3 p1 = (*projected)[vertex_indexes[i1]];
    vec3 p2 = (*projected)[vertex_indexes[i2]];

    i32 x02_len = p2.y - p0.y + 1;
    i32 x01_len = p1.y - p0.y + 1;
    i32 x12_len = p2.y - p1.y + 1;
    i32 x012_len = x01_len + x12_len - 1;

    edge_interpolate(p0.y, p0.x, p1.y, p1.x, p2.y, p2.x, x02, x012);
    edge_interpolate(p0.y, 1.0f / v0.z, p1.y, 1.0f / v1.z, p2.y, 1.0f / v2.z, iz02, iz012);

    i32 x02_middle = x02_len / 2;

    f64(*x_left)[];
    f64(*x_right)[];

    f64(*iz_left)[];
    f64(*iz_right)[];

    i32 left_len = 0, right_len = 0;

    if ((*x02)[x02_middle] < (*x012)[x02_middle])
    {
        x_left = x02;
        x_right = x012;

        iz_left = iz02;
        iz_right = iz012;

        left_len = x02_len;
        right_len = x012_len;
    }
    else
    {
        x_left = x012;
        x_right = x02;

        iz_left = iz012;
        iz_right = iz02;

        left_len = x012_len;
        right_len = x02_len;
    }

    for (i32 y = p0.y; y <= p2.y; y++)
    {
        i32 i_y = y - p0.y;
        i32 x_l = 0, x_r = 0, z_l = 0, z_r = 0;
        if (i_y <= left_len)
        {
            x_l = (*x_left)[(i32)(y - p0.y)];
            z_l = (*iz_left)[(i32)(y - p0.y)];
        }
        if (i_y <= right_len)
        {
            x_r = (*x_right)[(i32)(y - p0.y)];
            z_r = (*iz_right)[(i32)(y - p0.y)];
        }

        f64_interpolate(x_l, z_l, x_r, z_r, zscan);
        for (i32 x = x_l; x <= x_r; x++)
        {
            if (update_depth_buffer_if_closer(ctx, x, y, (*zscan)[(i32)(x - x_l)]))
            {
                ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, x), CENTER_TO_ZERO_Y(ctx.height, y), triangle.color.argb);
            }
        }
    }
}

void render_triangle(canvas ctx, triangle triangle, vec3 (*vertices)[], vec3 (*projected)[])
{
    draw_filled_triangle(ctx, triangle, vertices, projected);
}

void render_model(canvas ctx, model *model)
{
    memset(ctx.projected, 0, sizeof(vec3) * ctx.projected_size);
    for (i32 i = 0; i < ctx.projected_size; i++)
    {
        vec3 vertice = (*(model->vertices))[i];
        (*ctx.projected)[i] = vec4_project_vertex(ctx, (vec4){vertice.x, vertice.y, vertice.z, 1});
    }

    for (i32 i = 0; i < model->triangles_count; i++)
    {
        render_triangle(ctx, (*(model->triangles))[i], model->vertices, ctx.projected);
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
        {1, 5, 6, YELLOW},
        {1, 6, 2, YELLOW},
        {2, 6, 7, CYAN},
        {2, 7, 3, CYAN},
        {4, 0, 3, GREEN},
        {4, 1, 0, PURPLE},
        {4, 3, 7, GREEN},
        {4, 5, 1, PURPLE},
        {5, 4, 7, BLUE},
        {5, 7, 6, BLUE}};
    i32 triangles_count = 12;
    shuffle(&triangles, triangles_count);

    model cube = {
        &vertices,
        8,
        &triangles,
        triangles_count,
        {0, 0, 0},
        sqrt(3)};

    instance instances[] = {
        {&cube, {-1.5, 0, 7}, identity4x4, 0.75},
        {&cube, {1.25, 2.5, 7.5}, vec4_make_rotation_matrix(195), 1}};
    i32 instances_count = 2;
    for (i16 i = 0; i < instances_count; i++)
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
    memset(canvas.depth_buffer, 0, sizeof(f64) * canvas.width * canvas.height);
    render_scene(canvas, &camera, &instances, instances_count);
}
