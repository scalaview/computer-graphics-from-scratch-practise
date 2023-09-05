#include "canvas.h"
#include "logger.h"

static const vec3 camera_position = {0, 0, 0};
static const vec3 viewport_vector = {1, 1, 1};
const color backgroud_color = {.argb = 0xfffffff};
static RINLINE vec3 canvas_to_viewport(canvas ctx, i32 x, i32 y)
{
    return (vec3){
        x * viewport_vector.w / ctx.width,
        y * viewport_vector.h / ctx.height,
        viewport_vector.z};
}

static Bool intersect_ray_sphere(vec3 camera_position, vec3 viewport, sphere *sphere, f32 *t1, f32 *t2)
{
    vec3 co = vec3_sub(camera_position, sphere->center);
    f32 a = vec3_dot(viewport, viewport);
    f32 b = 2 * vec3_dot(co, viewport);
    f32 c = vec3_dot(co, co) - sphere->radius * sphere->radius;
    f32 discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return False;
    }

    *t1 = (-b + sqrt(discriminant)) / (2 * a);
    *t2 = (-b - sqrt(discriminant)) / (2 * a);
    return True;
}

static f32 compute_lighting(canvas ctx, vec3 point, vec3 normal, vec3 view, f32 specular)
{
    f32 intensity = 0.0f;
    for (i32 i = 0; i < ctx.light_size; i++)
    {
        light light = (*ctx.lights)[i];
        if (light.type == LIGHT_AMBIENT)
        {
            intensity += light.intensity;
        }
        else
        {
            vec3 l_vec;
            if (light.type == LIGHT_POINT)
                l_vec = vec3_sub(light.position, point);
            else
                l_vec = light.direction;
            // calculate cos(a)
            f32 n_dot_l = vec3_dot(normal, l_vec);
            if (n_dot_l > 0)
                intensity += light.intensity * n_dot_l / (vec3_length(normal) * vec3_length(l_vec));
            if (specular != -1)
            {
                // specular reflection
                vec3 reflect_vec = vec3_sub(vec3_mul_scalar(normal, 2 * vec3_dot(normal, l_vec)), l_vec);
                f32 r_dot_v = vec3_dot(reflect_vec, view);
                if (r_dot_v > 0)
                {
                    intensity += light.intensity * pow(r_dot_v / ((vec3_length(reflect_vec) * vec3_length(view))), specular);
                }
            }
        }
    }
    return intensity;
}

static Bool trace_ray(canvas ctx, vec3 camera_position, vec3 viewport, i32 min, i32 max, color *out_color)
{
    f32 closest_t = FLT_MAX;
    sphere *closest_sphere = 0;

    for (i32 i = 0; i < ctx.sphere_size; i++)
    {
        f32 t1, t2 = 0;
        sphere *sphere = &(*ctx.spheres)[i];
        Bool res = intersect_ray_sphere(camera_position, viewport, sphere, &t1, &t2);
        if (!res)
            continue;
        if (t1 > min && t1 < max && t1 < closest_t)
        {
            closest_t = t1;
            closest_sphere = sphere;
        }

        if (t2 > min && t2 < max && t2 < closest_t)
        {
            closest_t = t2;
            closest_sphere = sphere;
        }
    }
    if (closest_sphere)
    {
        vec3 intersection_point = vec3_add(camera_position, vec3_mul_scalar(viewport, closest_t));
        vec3 law = vec3_sub(intersection_point, closest_sphere->center);
        vec3 normal = vec3_normalized(law);
        f32 lighting_cost = compute_lighting(ctx, intersection_point, normal, vec3_mul_scalar(viewport, -1), closest_sphere->specular);
        *out_color = color_mul(closest_sphere->color, (vec3){lighting_cost, lighting_cost, lighting_cost});
        return True;
    }
    return False;
}

void render_frame(canvas ctx)
{
    for (i32 x = -ctx.width / 2; x < ctx.width / 2; x++)
    {
        for (i32 y = -ctx.height / 2; y < ctx.height / 2; y++)
        {
            vec3 viewport = canvas_to_viewport(ctx, x, y);
            color color = backgroud_color;
            trace_ray(ctx, camera_position, viewport, 1, INF, &color);
            ctx.put_pixel(CENTER_TO_ZERO_X(ctx.width, x), CENTER_TO_ZERO_Y(ctx.height, y), color.argb);
        }
    }
}