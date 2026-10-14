
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stddef.h>
#include <stdint.h>

/*
 * Read an entire file into memory.
 *
 * filename: Path to the input file
 * data:     Receives allocated buffer
 * length:   Receives file size in bytes
 *
 * Returns 0 on success, -1 on failure.
 *
 * Caller must free(*data).
 */
int file_read(const char *filename,
              uint8_t **data,
              size_t *length);

/*
 * Write binary data to a file.
 *
 * filename: Output file path
 * data:     Data to write
 * length:   Number of bytes to write
 *
 * Returns 0 on success, -1 on failure.
 */
int file_write(const char *filename,
               const uint8_t *data,
               size_t length);

/*
 * Check whether a file exists.
 *
 * Returns 1 if the file exists,
 * 0 otherwise.
 */
int file_exists(const char *filename);

#endif
