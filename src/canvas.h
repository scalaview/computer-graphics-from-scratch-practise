#pragma once

#include "rmath.h"
#include "define_types.h"
#include "color.h"

typedef struct sphere
{
    vec3 center;
    i32 radius;
    color color;
    f32 specular;
    f32 reflective;
} sphere;

typedef enum light_type
{
    LIGHT_AMBIENT = 0,
    LIGHT_POINT = 1,
    LIGHT_DIRECTIONAL = 2
} light_type;
typedef struct light
{
    light_type type;
    f32 intensity;
    union
    {
        vec3 position, direction;
    };

} light;

typedef struct canvas
{
    void (*put_pixel)(i32 x, i32 y, u32 color);
    i32 width;
    i32 height;
    sphere (*spheres)[];
    i32 sphere_size;

    light (*lights)[];
    i32 light_size;
} canvas;

void render_frame(canvas canvas);
extern const color backgroud_color;
#define CENTER_TO_ZERO_X(width, x) (width / 2 + x)
#define CENTER_TO_ZERO_Y(height, x) (height / 2 - y - 1)