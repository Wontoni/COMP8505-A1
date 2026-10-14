
#include "lsb.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#define HEADER_SIZE 4

/*
 * Number of usable color channels per pixel.
 * Alpha is excluded.
 */
static int usable_channels(const PNGImage *image)
{
    switch (image->color_type)
    {
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        return 1;

    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
        return 3;

    default:
        return 0;
    }
}

/*
 * Get a pointer to the Nth usable color channel.
 *
 * For RGBA, alpha is skipped.
 * For grayscale-alpha, alpha is skipped.
 */
static png_bytep get_channel(
    const PNGImage *image,
    size_t index)
{
    int usable = usable_channels(image);

    if (usable == 0)
        return NULL;

    size_t pixel_index = index / usable;
    size_t channel_index = index % usable;

    size_t x = pixel_index % image->width;
    size_t y = pixel_index / image->width;

    return &image->rows[y][x * image->channels + channel_index];
}

/*
 * Write one byte into eight consecutive LSBs.
 */
static void write_byte(
    PNGImage *image,
    size_t *bit_index,
    uint8_t value)
{
    for (int bit = 7; bit >= 0; bit--)
    {
        png_bytep channel = get_channel(
            image, (*bit_index)++);

        uint8_t current_bit = (value >> bit) & 1;

        *channel = (*channel & 0xFE) | current_bit;
    }
}

/*
 * Read one byte from eight consecutive LSBs.
 */
static uint8_t read_byte(
    const PNGImage *image,
    size_t *bit_index)
{
    uint8_t value = 0;

    for (int bit = 7; bit >= 0; bit--)
    {
        png_bytep channel = get_channel(
            image, (*bit_index)++);

        value = (value << 1) | (*channel & 1);
    }

    return value;
}

/*
 * Embed data into image.
 */
int lsb_encode(
    PNGImage *image,
    const uint8_t *data,
    size_t length)
{
    if (!image || !data || length == 0)
    {
        fprintf(stderr, "Error: Invalid LSB input\n");
        return -1;
    }

    if (length > UINT32_MAX)
    {
        fprintf(stderr, "Error: Message too large\n");
        return -1;
    }

    size_t capacity = png_capacity(image);

    if (capacity < HEADER_SIZE ||
        length > capacity - HEADER_SIZE)
    {
        fprintf(stderr,
                "Error: Message exceeds PNG capacity\n");
        return -1;
    }

    size_t bit_index = 0;

    // Store message length as 4-byte big-endian header
    uint32_t message_length = (uint32_t)length;

    for (int i = 3; i >= 0; i--)
    {
        uint8_t byte =
            (message_length >> (i * 8)) & 0xFF;

        write_byte(image, &bit_index, byte);
    }

    // Embed message bytes
    for (size_t i = 0; i < length; i++)
    {
        write_byte(image, &bit_index, data[i]);
    }

    return 0;
}

/*
 * Extract data from image.
 */
int lsb_decode(
    const PNGImage *image,
    uint8_t **data,
    size_t *length)
{
    if (!image || !data || !length)
    {
        fprintf(stderr, "Error: Invalid LSB input\n");
        return -1;
    }

    *data = NULL;
    *length = 0;

    size_t capacity = png_capacity(image);

    if (capacity < HEADER_SIZE)
    {
        fprintf(stderr, "Error: PNG too small\n");
        return -1;
    }

    size_t bit_index = 0;
    uint32_t message_length = 0;

    // Extract 4-byte message length
    for (int i = 0; i < HEADER_SIZE; i++)
    {
        message_length =
            (message_length << 8) |
            read_byte(image, &bit_index);
    }

    // Validate extracted length
    if (message_length == 0 ||
        message_length > capacity - HEADER_SIZE)
    {
        fprintf(stderr,
                "Error: Invalid embedded message length\n");
        return -1;
    }

    uint8_t *buffer = malloc(message_length);

    if (!buffer)
    {
        fprintf(stderr,
                "Error: Memory allocation failed\n");
        return -1;
    }

    // Extract message
    for (uint32_t i = 0; i < message_length; i++)
    {
        buffer[i] = read_byte(image, &bit_index);
    }

    *data = buffer;
    *length = message_length;

    return 0;
}
