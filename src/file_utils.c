
#include "file_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/*
 * Read a file into a dynamically allocated buffer.
 */
int file_read(const char *filename,
              uint8_t **data,
              size_t *length)
{
    if (!filename || !data || !length)
    {
        fprintf(stderr, "Error: Invalid file_read arguments\n");
        return -1;
    }

    *data = NULL;
    *length = 0;

    FILE *fp = fopen(filename, "rb");

    if (!fp)
    {
        perror("Error opening input file");
        return -1;
    }

    // Move to end to determine file size
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Error: Unable to seek file\n");
        fclose(fp);
        return -1;
    }

    long file_size = ftell(fp);

    if (file_size <= 0)
    {
        fprintf(stderr, "Error: File is empty or unreadable\n");
        fclose(fp);
        return -1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error: Unable to rewind file\n");
        fclose(fp);
        return -1;
    }

    // Allocate memory for file contents
    uint8_t *buffer = malloc((size_t)file_size);

    if (!buffer)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(fp);
        return -1;
    }

    // Read entire file
    size_t bytes_read = fread(
        buffer,
        1,
        (size_t)file_size,
        fp);

    if (bytes_read != (size_t)file_size)
    {
        fprintf(stderr, "Error: Failed to read entire file\n");
        free(buffer);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    *data = buffer;
    *length = bytes_read;

    return 0;
}

/*
 * Write binary data to a file.
 */
int file_write(const char *filename,
               const uint8_t *data,
               size_t length)
{
    if (!filename || !data || length == 0)
    {
        fprintf(stderr, "Error: Invalid file_write arguments\n");
        return -1;
    }

    FILE *fp = fopen(filename, "wb");

    if (!fp)
    {
        perror("Error opening output file");
        return -1;
    }

    size_t bytes_written = fwrite(
        data,
        1,
        length,
        fp);

    if (bytes_written != length)
    {
        fprintf(stderr, "Error: Failed to write entire file\n");
        fclose(fp);
        return -1;
    }

    if (fclose(fp) != 0)
    {
        perror("Error closing output file");
        return -1;
    }

    return 0;
}

/*
 * Check if a file exists.
 */
int file_exists(const char *filename)
{
    if (!filename)
        return 0;

    FILE *fp = fopen(filename, "rb");

    if (!fp)
        return 0;

    fclose(fp);
    return 1;
}
