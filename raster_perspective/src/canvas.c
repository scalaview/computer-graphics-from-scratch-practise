#include <string.h>

#include "canvas.h"
#include "model.h"
#include "logger.h"
#include <stdlib.h>

const color backgroud_color = {.argb = 0xffffff};
static const vec3 viewport_vector = {1, 1, 1};

#define STA_BUF_SIZE 450

Bool enabled_cull_face = true;
Bool draw_out_lines = false;
Bool use_vertex_normals = true;
shading_model shading_model_config = {.type = SM_PHONG};

i16 lighting_model = LM_DIFFUSE | LM_SPECULAR;

f64 (*bufv01)[];
f64 (*bufv12)[];
f64 (*bufv02)[];

f64 (*x02)[];
f64 (*x012)[];
f64 (*iz02)[];
f64 (*iz012)[];
f64 (*zscan)[];

f64 (*i02)[];
f64 (*i012)[];
f64 (*nx02)[];
f64 (*nx012)[];
f64 (*ny02)[];
f64 (*ny012)[];
f64 (*nz02)[];
f64 (*nz012)[];
f64 (*iscan)[];
f64 (*nxscan)[];
f64 (*nyscan)[];
f64 (*nzscan)[];

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

RINLINE vec2 canvas_to_viewport(i32 width, i32 height, f64 x, f64 y)
{
    return (vec2){x * viewport_vector.w / width,
                  y * viewport_vector.w / height};
}

#define PROJECT_VERTEX_HELPER(n) n##_project_vertex
#define PROJECT_VERTEX_DEF(t)                                                                                               \
    RINLINE vec3 PROJECT_VERTEX_HELPER(t)(canvas ctx, t v)                                                                  \
    {                                                                                                                       \
        vec2 v2 = view_port_to_canvas(ctx.width, ctx.height, v.x * viewport_vector.z / v.z, v.y * viewport_vector.z / v.z); \
        return vec2_to_vec3(v2);                                                                                            \
    }

PROJECT_VERTEX_DEF(vec3);
PROJECT_VERTEX_DEF(vec4);

void shuffle(triangle (*triangles)[], i32 count)
{
    for (i32 i = count - 1; i > 0; i--)
    {
        i32 idx = rand() % (i + 1);
        triangle t = (*triangles)[idx];
        (*triangles)[idx] = (*triangles)[i];
        (*triangles)[i] = t;
    }
}

f64 compute_illumination(vec3 vertex, vec3 normal, camera *camera, light (*lights)[], i32 lights_count)
{
    f64 illumination = 0;
    for (i32 l = 0; l < lights_count; l++)
    {
        light light = (*lights)[l];
        if (light.type == LIGHT_AMBIENT)
        {
            illumination += light.intensity;
            continue;
        }

        vec3 vl;
        if (light.type == LIGHT_DIRECTIONAL)
        {
            mat4x4 camera_m = vec4_transpose(camera->orientation);
            // rotated light
            vl = vec4_to_vec3(vec4_mul_mv(camera_m, vec3_to_vec4(light.position)));
        }
        else if (light.type == LIGHT_POINT)
        {
            mat4x4 camera_m = make_camera_matrix(camera);
            vec3 transform_light = vec4_to_vec3(vec4_mul_mv(camera_m, vec3_to_vec4(light.position)));
            vl = vec3_add(transform_light, vec3_mul_scalar(vertex, -1.0));
        }

        //  diffuse component
        if (lighting_model & LM_DIFFUSE)
        {
            f64 cos_alpha = vec3_dot(vl, normal) / (vec3_length(vl) * vec3_length(normal));
            if (cos_alpha > 0)
            {
                illumination += cos_alpha * light.intensity;
            }
        }

        // specular component
        if (lighting_model & LM_SPECULAR)
        {
            vec3 reflected = vec3_add(vec3_mul_scalar(normal, 2 * vec3_dot(normal, vl)), vec3_mul_scalar(vl, -1));
            vec3 view = vec3_add(camera->position, vec3_mul_scalar(vertex, -1));

            f64 cos_beta = vec3_dot(reflected, view) / (vec3_length(reflected) * vec3_length(view));
            if (cos_beta > 0)
            {
                f64 specular = 50;
                illumination += pow(cos_beta, specular) * light.intensity;
            }
        }
    }

    return illumination;
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

RINLINE vec3 un_project_vertex(canvas canvas, f64 x, f64 y, f64 inv_z)
{
    f64 oz = 1.0 / inv_z;
    f64 ux = x * oz / viewport_vector.z;
    f64 uy = y * oz / viewport_vector.z;
    vec2 p2d = canvas_to_viewport(canvas.width, canvas.height, ux, uy);
    return (vec3){p2d.x, p2d.y, oz};
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

void draw_filled_triangle(canvas ctx, triangle triangle, vec3 (*vertices)[], vec3 (*projected)[], camera *camera, light (*lights)[], i32 lights_count, mat4x4 orientation)
{
    if (triangle.p1 == 22 && triangle.p2 == 38 && triangle.p3 == 23)
    {
        i32 a = 1;
    }
    vec3 normal = compute_triangle_normal((*vertices)[triangle.p1], (*vertices)[triangle.p2], (*vertices)[triangle.p3]);
    vec3 vertex_to_camera = vec3_mul_scalar((*vertices)[triangle.p1], -1);
    if (enabled_cull_face && !cull_back_faces(&vertex_to_camera, &normal))
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

    i32 len_02 = p2.y - p0.y + 1;
    i32 len_01 = p1.y - p0.y + 1;
    i32 len_12 = p2.y - p1.y + 1;
    i32 len_012 = len_01 + len_12 - 1;

    edge_interpolate(p0.y, p0.x, p1.y, p1.x, p2.y, p2.x, x02, x012);
    edge_interpolate(p0.y, 1.0f / v0.z, p1.y, 1.0f / v1.z, p2.y, 1.0f / v2.z, iz02, iz012);

    vec3 normal0 = normal, normal1 = normal, normal2 = normal;
    if (use_vertex_normals)
    {
        mat4x4 transform = vec4_mul_mm(vec4_transpose(camera->orientation), orientation);
        normal0 = vec4_to_vec3(vec4_mul_mv(transform, vec3_to_vec4(triangle.normals[i0])));
        normal1 = vec4_to_vec3(vec4_mul_mv(transform, vec3_to_vec4(triangle.normals[i1])));
        normal2 = vec4_to_vec3(vec4_mul_mv(transform, vec3_to_vec4(triangle.normals[i2])));
    }

    f64 intensity __attribute__((unused)) = 1;
    if (shading_model_config.type == SM_FLAT)
    {
        // flat shading
        vec3 center = {.x = (v0.x + v1.x + v2.x) / 3, .y = (v0.y + v1.y + v2.y) / 3, .z = (v0.z + v1.z + v2.z) / 3};
        intensity = compute_illumination(center, normal0, camera, lights, lights_count);
    }
    else if (shading_model_config.type == SM_GOURAUD)
    {
        f64 i0 = compute_illumination(v0, normal0, camera, lights, lights_count);
        f64 i1 = compute_illumination(v1, normal1, camera, lights, lights_count);
        f64 i2 = compute_illumination(v2, normal2, camera, lights, lights_count);

        edge_interpolate(p0.y, i0, p1.y, i1, p2.y, i2, i02, i012);
    }
    else if (shading_model_config.type == SM_PHONG)
    {
        edge_interpolate(p0.y, normal0.x, p1.y, normal1.x, p2.y, normal2.x, nx02, nx012);
        edge_interpolate(p0.y, normal0.y, p1.y, normal1.y, p2.y, normal2.y, ny02, ny012);
        edge_interpolate(p0.y, normal0.z, p1.y, normal1.z, p2.y, normal2.z, nz02, nz012);
    }

    i32 middle = len_02 / 2;

    f64(*x_left)[];
    f64(*x_right)[];

    f64(*iz_left)[];
    f64(*iz_right)[];

    f64(*i_left)[];
    f64(*i_right)[];

    f64(*nx_left)[];
    f64(*nx_right)[];

    f64(*ny_left)[];
    f64(*ny_right)[];

    f64(*nz_left)[];
    f64(*nz_right)[];

    i32 left_len = 0, right_len = 0;

    if ((*x02)[middle] < (*x012)[middle])
    {
        x_left = x02;
        x_right = x012;

        iz_left = iz02;
        iz_right = iz012;

        i_left = i02;
        i_right = i012;

        nx_left = nx02;
        nx_right = nx012;

        ny_left = ny02;
        ny_right = ny012;

        nz_left = nz02;
        nz_right = nz012;

        left_len = len_02;
        right_len = len_012;
    }
    else
    {
        x_left = x012;
        x_right = x02;

        iz_left = iz012;
        iz_right = iz02;

        i_left = i012;
        i_right = i02;

        nx_left = nx012;
        nx_right = nx02;

        ny_left = ny012;
        ny_right = ny02;

        nz_left = nz012;
        nz_right = nz02;

        left_len = len_012;
        right_len = len_02;
    }

    for (i32 y = p0.y; y <= p2.y; y++)
    {
        i32 i_y = y - p0.y;
        f64 x_l = 0, x_r = 0, nx_l = 0, nx_r = 0, ny_l = 0, ny_r = 0, z_l = 0, z_r = 0, i_l = 0, i_r = 0, nz_l = 0, nz_r = 0;
        if (i_y <= left_len)
        {
            x_l = (*x_left)[i_y];
            z_l = (*iz_left)[i_y];
        }
        if (i_y <= right_len)
        {
            x_r = (*x_right)[i_y];
            z_r = (*iz_right)[i_y];
        }

        f64_interpolate(x_l, z_l, x_r, z_r, zscan);

        if (shading_model_config.type == SM_GOURAUD)
        {
            i_l = (*i_left)[i_y];
            i_r = (*i_right)[i_y];
            f64_interpolate(x_l, i_l, x_r, i_r, iscan);
        }
        else if (shading_model_config.type == SM_PHONG)
        {
            nx_l = (*nx_left)[i_y];
            nx_r = (*nx_right)[i_y];

            ny_l = (*ny_left)[i_y];
            ny_r = (*ny_right)[i_y];

            nz_l = (*nz_left)[i_y];
            nz_r = (*nz_right)[i_y];

            f64_interpolate(x_l, nx_l, x_r, nx_r, nxscan);
            f64_interpolate(x_l, ny_l, x_r, ny_r, nyscan);
            f64_interpolate(x_l, nz_l, x_r, nz_r, nzscan);
        }

        for (i32 x = x_l; x <= x_r; x++)
        {
            i32 i_x = x - x_l;
            f64 inv_z = (*zscan)[i_x];
            if (update_depth_buffer_if_closer(ctx, x, y, inv_z))
            {
                if (shading_model_config.type == SM_FLAT)
                {
                    // Just use the per-triangle intensity.
                }
                else if (shading_model_config.type == SM_GOURAUD)
                {
                    intensity = (*iscan)[i_x];
                }
                else if (shading_model_config.type == SM_PHONG)
                {
                    vec3 vertex = un_project_vertex(ctx, x, y, inv_z);
                    vec3 normal = (vec3){(*nxscan)[i_x], (*nyscan)[i_x], (*nzscan)[i_x]};
                    intensity = compute_illumination(vertex, normal, camera, lights, lights_count);
                }
                ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, x), CENTER_TO_ZERO_Y(ctx.height, y), color_mul_scalar(triangle.color, intensity).argb);
            }
        }
    }

    // draw outlines
    if (draw_out_lines)
    {
        color outline_color = color_mul_scalar(triangle.color, 0.75f);
        draw_line(ctx, p0, p1, outline_color);
        draw_line(ctx, p1, p2, outline_color);
        draw_line(ctx, p2, p0, outline_color);
    }
}

void render_triangle(canvas ctx, triangle triangle, vec3 (*vertices)[], vec3 (*projected)[], camera *camera, light (*lights)[], i32 lights_count, mat4x4 orientation)
{
    draw_filled_triangle(ctx, triangle, vertices, projected, camera, lights, lights_count, orientation);
}

void render_model(canvas ctx, model *model, camera *camera, light (*lights)[], i32 lights_count, mat4x4 orientation, vec3 (*projected)[])
{
    memset(projected, 0, sizeof(vec3) * model->vertices_count);
    for (i32 i = 0; i < model->vertices_count; i++)
    {
        vec3 vertice = (*(model->vertices))[i];
        (*projected)[i] = vec4_project_vertex(ctx, vec3_to_vec4(vertice));
    }

    for (i32 i = 0; i < model->triangles_count; i++)
    {
        render_triangle(ctx, (*(model->triangles))[i], model->vertices, projected, camera, lights, lights_count, orientation);
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
    vec4 center_w = vec4_mul_mv(transform, vec3_to_vec4(origin_model->bounds_center));
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
        vec4 vertice_transformed = vec4_mul_mv(transform, vec3_to_vec4(vertice));
        (*(clipped->vertices))[i] = vec4_to_vec3(vertice_transformed);
    }

    memcpy(origin_model->transform_and_clip_triangles, origin_model->triangles, sizeof(triangle) * origin_model->triangles_count);
    i32 triangles_count = origin_model->triangles_count;
    for (i32 i = 0; i < clipping_plane_count; i++)
    {
        i32 idx = 0;
        for (i32 j = 0; j < origin_model->triangles_count; j++)
        {
            triangle triangle = (*origin_model->triangles)[j];
            if (clip_triangle(ctx, &(*origin_model->triangles)[j], &(*clipping_planes)[i], clipped->vertices, clipped->vertices_count, &((*origin_model->transform_and_clip_new_triangles)[idx])))
            {
                idx++;
            }
        }
        if (idx > 0)
        {
            memcpy(origin_model->transform_and_clip_triangles, origin_model->transform_and_clip_new_triangles, sizeof(triangle) * idx);
            triangles_count = idx;
        }
    }
    memcpy(clipped->triangles, origin_model->transform_and_clip_triangles, sizeof(triangle) * triangles_count);
    clipped->triangles_count = triangles_count;
    clipped->bounds_center = center;
    clipped->bounds_radius = radius;
    clipped->vertices_count = origin_model->vertices_count;
    return True;
}

void render_scene(canvas ctx, camera *camera, instance (*instances)[], i32 instance_size, light (*lights)[], i32 lights_count)
{
    mat4x4 m_camera = make_camera_matrix(camera);
    for (i32 i = 0; i < instance_size; i++)
    {
        instance instance = (*instances)[i];
        model *t_model = instance.model;
        mat4x4 transform = vec4_mul_mm(m_camera, instance.transform);

        memset(t_model->render_scene_triangles, 0, sizeof(triangle) * t_model->triangles_count);
        memset(t_model->render_scene_vertexs, 0, sizeof(vec3) * t_model->vertices_count);
        model clipped = {.vertices = t_model->render_scene_vertexs, .triangles = t_model->render_scene_triangles};
        if (transform_and_clip(ctx, camera->clipping_planes, camera->clipping_plane_count, instance.model, instance.scale, transform, &clipped))
            render_model(ctx, &clipped, camera, lights, lights_count, instance.orientation, instance.projected);
    }
}

void render_frame(canvas canvas)
{
    extern model cube;
    extern model sphere;
    extern instance(*instances)[];
    extern i32 instances_count;

    f64 sqrt2 = 1 / sqrt(2);
    plane clipping_planes[] = {
        {(vec3){0, 0, 1}, -1},         // Near
        {(vec3){sqrt2, 0, sqrt2}, 0},  // Left
        {(vec3){-sqrt2, 0, sqrt2}, 0}, // Right
        {(vec3){0, -sqrt2, sqrt2}, 0}, // Top
        {(vec3){0, sqrt2, sqrt2}, 0},  // Bottom
    };
    camera camera = {{-3, 1, 2}, vec4_make_rotation_matrix(-30), &clipping_planes, 5};

    light lights[3] = {
        {.type = LIGHT_AMBIENT, .intensity = 0.2},
        {.type = LIGHT_DIRECTIONAL, .intensity = 0.2, .position = (vec3){.x = -1, .y = 0, .z = 1}},
        {.type = LIGHT_POINT, .intensity = 0.6, .position = (vec3){.x = -3, .y = 2, .z = -10}}};
    i32 lights_count = 3;
    memset(canvas.depth_buffer, 0, sizeof(f64) * canvas.width * canvas.height);
    render_scene(canvas, &camera, instances, instances_count, &lights, lights_count);
}

void prepare_memory_buffer(canvas canvas_ctx)
{
    bufv01 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    bufv12 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    bufv02 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);

    x02 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    x012 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    iz02 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    iz012 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    zscan = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    iscan = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    nxscan = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    nyscan = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    nzscan = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);

    i02 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    i012 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    nx02 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    nx012 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    ny02 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    ny012 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    nz02 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
    nz012 = malloc(sizeof(f64) * canvas_ctx.width * canvas_ctx.height);
}

void free_memory_buffer()
{
    free(bufv01);
    free(bufv12);
    free(bufv02);
    free(x02);
    free(x012);
    free(iz02);
    free(iz012);
    free(zscan);
    free(i02);
    free(i012);
    free(nx02);
    free(nx012);
    free(ny02);
    free(ny012);
    free(nz02);
    free(nz012);
    free(iscan);
    free(nxscan);
    free(nyscan);
    free(nzscan);
}