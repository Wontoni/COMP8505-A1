
#ifndef LSB_H
#define LSB_H

#include <stddef.h>
#include <stdint.h>

#include "png_handler.h"

/*
 * Embed binary data into a PNG image.
 *
 * image:   PNG image loaded into memory
 * data:    Data to embed
 * length:  Number of bytes to embed
 *
 * Returns 0 on success, -1 on failure.
 */
int lsb_encode(PNGImage *image,
               const uint8_t *data,
               size_t length);

/*
 * Extract binary data from a PNG image.
 *
 * image:   PNG image containing hidden data
 * data:    Pointer receiving allocated data
 * length:  Pointer receiving extracted length
 *
 * Returns 0 on success, -1 on failure.
 *
 * Caller must free(*data).
 */
int lsb_decode(const PNGImage *image,
               uint8_t **data,
               size_t *length);

#endif
