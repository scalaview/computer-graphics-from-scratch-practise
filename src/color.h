#pragma once
#include "define_types.h"

typedef union color
{

    u32 argb : 32;
    struct
    {
        u8 b : 8;
        u8 g : 8;
        u8 r : 8;
        u8 alpha : 8;
    } ATTR_PACKED;
} color;
