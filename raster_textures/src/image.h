#pragma once

#include "define_types.h"

typedef struct image
{
    i32 width;
    i32 height;
    i32 channels;
    u8 *data;
} image;

Bool read_image_from_file(const char *filename, image *image);

void free_image(image *image);
