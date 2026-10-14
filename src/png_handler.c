
#include "png_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
 * Load PNG image into memory.
 */
PNGImage *png_load(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        perror("Error opening PNG");
        return NULL;
    }

    // Validate PNG signature
    png_byte header[8];

    if (fread(header, 1, 8, fp) != 8 ||
        png_sig_cmp(header, 0, 8))
    {
        fprintf(stderr, "Error: Invalid PNG file\n");
        fclose(fp);
        return NULL;
    }

    PNGImage *image = calloc(1, sizeof(PNGImage));

    if (!image)
    {
        fclose(fp);
        return NULL;
    }

    image->png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr)
        goto error;

    image->info_ptr = png_create_info_struct(image->png_ptr);

    if (!image->info_ptr)
        goto error;

    if (setjmp(png_jmpbuf(image->png_ptr)))
        goto error;

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);

    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(
        image->png_ptr, image->info_ptr);

    image->height = png_get_image_height(
        image->png_ptr, image->info_ptr);

    image->bit_depth = png_get_bit_depth(
        image->png_ptr, image->info_ptr);

    image->color_type = png_get_color_type(
        image->png_ptr, image->info_ptr);

    // Normalize supported images to 8-bit channels
    if (image->bit_depth == 16)
        png_set_strip_16(image->png_ptr);

    if (image->color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(image->png_ptr);

    if (image->color_type == PNG_COLOR_TYPE_GRAY &&
        image->bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(image->png_ptr);

    // Convert transparency chunk to alpha
    if (png_get_valid(image->png_ptr,
                      image->info_ptr,
                      PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(image->png_ptr);

    // Support interlaced images
    png_set_interlace_handling(image->png_ptr);

    png_read_update_info(image->png_ptr, image->info_ptr);

    image->bit_depth = png_get_bit_depth(
        image->png_ptr, image->info_ptr);

    image->color_type = png_get_color_type(
        image->png_ptr, image->info_ptr);

    image->channels = png_get_channels(
        image->png_ptr, image->info_ptr);

    image->rowbytes = png_get_rowbytes(
        image->png_ptr, image->info_ptr);

    if (image->height > SIZE_MAX / sizeof(png_bytep))
        goto error;

    image->rows = calloc(image->height, sizeof(png_bytep));

    if (!image->rows)
        goto error;

    for (png_uint_32 y = 0; y < image->height; y++)
    {
        image->rows[y] = malloc(image->rowbytes);

        if (!image->rows[y])
            goto error;
    }

    png_read_image(image->png_ptr, image->rows);
    png_read_end(image->png_ptr, NULL);

    fclose(fp);
    return image;

error:
    fprintf(stderr, "Error: Failed to load PNG\n");
    fclose(fp);
    free_image_png(image);
    return NULL;
}

/*
 * Save modified PNG image.
 */
int png_save(const char *filename, PNGImage *image)
{
    if (!image || !image->rows)
        return -1;

    FILE *fp = fopen(filename, "wb");

    if (!fp)
    {
        perror("Error creating PNG");
        return -1;
    }

    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(
        png_ptr,
        info_ptr,
        image->width,
        image->height,
        image->bit_depth,
        image->color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, image->rows);
    png_write_end(png_ptr, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}

/*
 * Calculate usable LSB capacity in bytes.
 * Alpha channels are excluded.
 */
size_t png_capacity(const PNGImage *image)
{
    if (!image)
        return 0;

    size_t channels;

    switch (image->color_type)
    {
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        channels = 1;
        break;

    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
        channels = 3;
        break;

    default:
        return 0;
    }

    size_t pixels = (size_t)image->width * image->height;

    if (image->width != 0 &&
        pixels / image->width != image->height)
        return 0;

    if (pixels > SIZE_MAX / channels)
        return 0;

    return (pixels * channels) / 8;
}

/*
 * Free PNG resources.
 */
void free_image_png(PNGImage *image)
{
    if (!image)
        return;

    if (image->rows)
    {
        for (png_uint_32 y = 0; y < image->height; y++)
            free(image->rows[y]);

        free(image->rows);
    }

    if (image->png_ptr)
    {
        png_destroy_read_struct(
            &image->png_ptr,
            &image->info_ptr,
            NULL);
    }

    free(image);
}
