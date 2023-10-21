#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Bool read_image_from_file(const char *filename, image *image)
{
    image->data = stbi_load(filename, &image->width, &image->height, &image->channels, 0);
    if (image->data == NULL)
    {
        return False;
    }
    return True;
}

void free_image(image *image)
{
    stbi_image_free(image->data);
}
