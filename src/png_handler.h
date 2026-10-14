
#ifndef PNG_HANDLER_H
#define PNG_HANDLER_H

#include <png.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    png_uint_32 width;
    png_uint_32 height;

    int bit_depth;
    int color_type;
    int channels;

    size_t rowbytes;
    png_bytep *rows;

    // Original PNG metadata
    png_structp png_ptr;
    png_infop info_ptr;
} PNGImage;

/*
 * Reads a PNG file into memory.
 * Returns NULL on failure.
 */
PNGImage *png_load(const char *filename);

/*
 * Saves a PNG image to a file.
 * Returns 0 on success, -1 on failure.
 */
int png_save(const char *filename, PNGImage *image);

/*
 * Calculates the number of bytes available
 * for LSB embedding, excluding alpha channels.
 */
size_t png_capacity(const PNGImage *image);

/*
 * Releases all memory associated with an image.
 */
void free_image_png(PNGImage *image);

#endif
