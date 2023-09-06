#include "canvas.h"
#include "logger.h"

static const vec3 camera_position = {0, 0, 0};
static const vec3 viewport_vector = {1, 1, 1};
const color backgroud_color = {.argb = 0x0};
static RINLINE vec3 canvas_to_viewport(canvas ctx, i32 x, i32 y)
{
    return (vec3){
        x * viewport_vector.w / ctx.width,
        y * viewport_vector.h / ctx.height,
        viewport_vector.z};
}

RINLINE vec3 reflect_ray(vec3 r_vec, vec3 normal)
{
    return vec3_sub(vec3_mul_scalar(normal, 2 * vec3_dot(normal, r_vec)), r_vec);
}

static Bool intersect_ray_sphere(vec3 camera_position, vec3 viewport, sphere *sphere, f64 *t1, f64 *t2)
{
    vec3 co = vec3_sub(camera_position, sphere->center);
    f64 a = vec3_dot(viewport, viewport);
    f64 b = 2 * vec3_dot(co, viewport);
    f64 c = vec3_dot(co, co) - sphere->radius * sphere->radius;
    f64 discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return False;
    }

    *t1 = (-b + sqrt(discriminant)) / (2 * a);
    *t2 = (-b - sqrt(discriminant)) / (2 * a);
    return True;
}

static Bool closest_intersection(canvas ctx, vec3 camera_position, vec3 viewport, f64 t_min, f64 t_max, f64 *out_closest_t, sphere **out_closest_sphere)
{
    f64 closest_t = FLT_MAX;
    sphere *closest_sphere = 0;

    for (i32 i = 0; i < ctx.sphere_size; i++)
    {
        f64 t1, t2 = 0;
        sphere *sphere = &(*ctx.spheres)[i];
        Bool res = intersect_ray_sphere(camera_position, viewport, sphere, &t1, &t2);
        if (!res)
            continue;
        if (t1 > t_min && t1 < t_max && t1 < closest_t)
        {
            closest_t = t1;
            closest_sphere = sphere;
        }

        if (t2 > t_min && t2 < t_max && t2 < closest_t)
        {
            closest_t = t2;
            closest_sphere = sphere;
        }
    }
    if (closest_sphere)
    {
        *out_closest_t = closest_t;
        *out_closest_sphere = closest_sphere;
        return True;
    }
    return False;
}

static f64 compute_lighting(canvas ctx, vec3 point, vec3 normal, vec3 view, f64 specular)
{
    f32 intensity = 0.0f;
    f32 t_min = 0.001f;
    for (i32 i = 0; i < ctx.light_size; i++)
    {
        light light = (*ctx.lights)[i];
        if (light.type == LIGHT_AMBIENT)
        {
            intensity += light.intensity;
        }
        else
        {
            f32 t_max;
            vec3 l_vec;
            if (light.type == LIGHT_POINT)
            {
                l_vec = vec3_sub(light.position, point);
                t_max = 1.0;
            }
            else
            {
                l_vec = light.direction;
                t_max = FLT_MAX;
            }
            // calculate shadow
            f64 closest_t = FLT_MAX;
            sphere *closest_sphere = 0;
            if (closest_intersection(ctx, point, l_vec, t_min, t_max, &closest_t, &closest_sphere))
                continue;

            // calculate cos(a)
            f64 n_dot_l = vec3_dot(normal, l_vec);
            if (n_dot_l > 0)
                intensity += light.intensity * n_dot_l / (vec3_length(normal) * vec3_length(l_vec));
            if (specular != -1)
            {
                // specular reflection
                vec3 reflect_vec = reflect_ray(l_vec, normal);
                f64 r_dot_v = vec3_dot(reflect_vec, view);
                if (r_dot_v > 0)
                {
                    intensity += light.intensity * pow(r_dot_v / ((vec3_length(reflect_vec) * vec3_length(view))), specular);
                }
            }
        }
    }
    return intensity;
}

static Bool trace_ray(canvas ctx, vec3 camera_position, vec3 viewport, f64 t_min, f64 t_max, i32 recursion_depth, color *out_color)
{
    f64 closest_t = FLT_MAX;
    sphere *closest_sphere = 0;
    f32 min_t = 0.001f;
    if (closest_intersection(ctx, camera_position, viewport, t_min, t_max, &closest_t, &closest_sphere))
    {
        vec3 intersection_point = vec3_add(camera_position, vec3_mul_scalar(viewport, closest_t));
        vec3 law = vec3_sub(intersection_point, closest_sphere->center);
        vec3 normal = vec3_normalized(law);
        vec3 view_vec = vec3_mul_scalar(viewport, -1);
        f64 lighting_cost = compute_lighting(ctx, intersection_point, normal, view_vec, closest_sphere->specular);
        *out_color = color_mul_scalar(closest_sphere->color, lighting_cost);

        // calculate reflective ray
        f32 reflective = closest_sphere->reflective;
        if (recursion_depth <= 0 || reflective <= 0)
            return True;

        vec3 reflect_ray_vec = reflect_ray(view_vec, normal);
        color reflected_color = backgroud_color;
        trace_ray(ctx, intersection_point, reflect_ray_vec, min_t, FLT_MAX, recursion_depth - 1, &reflected_color);
        *out_color = color_add(color_mul_scalar(*out_color, 1 - reflective), color_mul_scalar(reflected_color, reflective));
        return True;
    }
    return False;
}

void render_frame(canvas ctx)
{
    i32 recursion_depth = 3;
    for (i32 x = -ctx.width / 2; x < ctx.width / 2; x++)
    {
        for (i32 y = -ctx.height / 2; y < ctx.height / 2; y++)
        {
            vec3 viewport = canvas_to_viewport(ctx, x, y);
            color color = backgroud_color;
            trace_ray(ctx, camera_position, viewport, 1.0f, FLT_MAX, recursion_depth, &color);
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, x), CENTER_TO_ZERO_Y(ctx.height, y), color.argb);
        }
    }
}