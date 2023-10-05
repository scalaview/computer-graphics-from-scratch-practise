#pragma once
#include "rmath.h"
#include "define_types.h"
#include "color.h"

typedef struct triangle
{
    u32 p1, p2, p3;
    color color;
} triangle;

typedef struct model
{
    vec3 (*vertices)[];
    triangle (*triangles)[];
} model;

typedef struct instance
{
    model *model;
    vec3 position;
} instance;