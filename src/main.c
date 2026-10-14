
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "png_handler.h"
#include "lsb.h"
#include "otp.h"
#include "file_utils.h"

/*
 * Display command-line usage.
 */
static void print_usage(const char *program)
{
    printf("PNG LSB Steganography Tool\n\n");

    printf("Usage:\n");
    printf("  %s encode <original.png> <message.txt> <key.txt> <carrier.png>\n",
           program);
    printf("  %s decode <carrier.png> <.txt/.bin> <output.txt>\n",
           program);
    printf("  %s keygen <length> <key.bin>\n",
           program);
}

/*
 * Encode a plaintext message into a PNG.
 */
static int encode(const char *cover_path,
                  const char *message_path,
                  const char *key_path,
                  const char *output_path)
{
    PNGImage *image = NULL;

    uint8_t *plaintext = NULL;
    uint8_t *key = NULL;
    uint8_t *ciphertext = NULL;

    size_t message_length = 0;
    size_t key_length = 0;

    int result = 1;

    // Prevent accidental output overwrites
    if (file_exists(output_path))
    {
        fprintf(stderr,
                "Error: Output file already exists: %s\n",
                output_path);
        return 1;
    }

    // Read plaintext message
    if (file_read(message_path,
                  &plaintext,
                  &message_length) != 0)
        goto cleanup;

    printf("[+] Message loaded: %zu bytes\n",
           message_length);

    // Read user-supplied OTP key
    if (file_read(key_path,
                  &key,
                  &key_length) != 0)
        goto cleanup;

    // Validate key length
    if (key_length < message_length)
    {
        fprintf(stderr,
                "Error: OTP key is too short.\n"
                "Required: %zu bytes, Available: %zu bytes\n",
                message_length, key_length);
        goto cleanup;
    }

    // Load cover PNG
    image = png_load(cover_path);

    if (!image)
        goto cleanup;

    printf("[+] PNG loaded: %u x %u\n",
           image->width, image->height);

    // Check capacity (4 bytes reserved for length header)
    size_t capacity = png_capacity(image);

    if (capacity < 4 ||
        message_length > capacity - 4)
    {
        fprintf(stderr,
                "Error: Message exceeds PNG capacity.\n");
        goto cleanup;
    }

    printf("[+] Available payload capacity: %zu bytes\n",
           capacity - 4);

    // Allocate ciphertext buffer
    ciphertext = malloc(message_length);

    if (!ciphertext)
    {
        fprintf(stderr,
                "Error: Memory allocation failed\n");
        goto cleanup;
    }

    // Encrypt using OTP
    if (otp_xor(plaintext,
                key,
                ciphertext,
                message_length) != 0)
        goto cleanup;

    printf("[+] Message encrypted successfully\n");

    // Embed encrypted data using LSB
    if (lsb_encode(image,
                   ciphertext,
                   message_length) != 0)
        goto cleanup;

    printf("[+] Ciphertext embedded successfully\n");

    // Save stego image
    if (png_save(output_path, image) != 0)
    {
        fprintf(stderr,
                "Error: Failed to save stego image\n");
        goto cleanup;
    }

    printf("[+] Stego image saved: %s\n",
           output_path);

    result = 0;

cleanup:
    free(plaintext);
    free(key);
    free(ciphertext);
    free_image_png(image);

    return result;
}

/*
 * Decode and decrypt an embedded message.
 */
static int decode(const char *stego_path,
                  const char *key_path,
                  const char *output_path)
{
    PNGImage *image = NULL;

    uint8_t *ciphertext = NULL;
    uint8_t *key = NULL;
    uint8_t *plaintext = NULL;

    size_t message_length = 0;
    size_t key_length = 0;

    int result = 1;

    // Prevent overwriting existing output
    if (file_exists(output_path))
    {
        fprintf(stderr,
                "Error: Output file already exists: %s\n",
                output_path);
        return 1;
    }

    // Load stego PNG
    image = png_load(stego_path);

    if (!image)
        goto cleanup;

    printf("[+] Stego PNG loaded: %u x %u\n",
           image->width, image->height);

    // Extract encrypted data
    if (lsb_decode(image,
                   &ciphertext,
                   &message_length) != 0)
        goto cleanup;

    printf("[+] Extracted ciphertext: %zu bytes\n",
           message_length);

    // Load OTP key
    if (file_read(key_path,
                  &key,
                  &key_length) != 0)
        goto cleanup;

    // Validate key length
    if (key_length < message_length)
    {
        fprintf(stderr,
                "Error: OTP key is too short.\n");
        goto cleanup;
    }

    // Allocate plaintext buffer
    plaintext = malloc(message_length);

    if (!plaintext)
    {
        fprintf(stderr,
                "Error: Memory allocation failed\n");
        goto cleanup;
    }

    // Decrypt using OTP
    if (otp_xor(ciphertext,
                key,
                plaintext,
                message_length) != 0)
        goto cleanup;

    printf("[+] Message decrypted successfully\n");

    // Save decrypted plaintext
    if (file_write(output_path,
                   plaintext,
                   message_length) != 0)
        goto cleanup;

    printf("[+] Decrypted message saved: %s\n",
           output_path);

    // Display plaintext to console
    printf("\nDecoded message:\n");
    fwrite(plaintext, 1, message_length, stdout);
    printf("\n");

    result = 0;

cleanup:
    free(ciphertext);
    free(key);
    free(plaintext);
    free_image_png(image);

    return result;
}

/*
 * Generate a random OTP key file.
 */
static int keygen(const char *length_str,
                  const char *output_path)
{
    char *endptr;
    errno = 0;

    unsigned long long requested =
        strtoull(length_str, &endptr, 10);

    if (errno != 0 ||
        length_str[0] == '\0' ||
        length_str[0] == '-' ||
        *endptr != '\0' ||
        requested == 0 ||
        requested > SIZE_MAX)
    {
        fprintf(stderr,
                "Error: Invalid key length\n");
        return 1;
    }

    size_t length = (size_t)requested;

    if (file_exists(output_path))
    {
        fprintf(stderr,
                "Error: Key file already exists: %s\n",
                output_path);
        return 1;
    }

    uint8_t *key = malloc(length);

    if (!key)
    {
        fprintf(stderr,
                "Error: Memory allocation failed\n");
        return 1;
    }

    // Generate random key
    if (otp_generate_key(key, length) != 0)
    {
        free(key);
        return 1;
    }

    // Save key
    if (file_write(output_path, key, length) != 0)
    {
        free(key);
        return 1;
    }

    printf("[+] OTP key generated: %zu bytes\n",
           length);
    printf("[+] Key saved: %s\n",
           output_path);

    free(key);
    return 0;
}

/*
 * Main command-line interface.
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "encode") == 0)
    {
        if (argc != 6)
        {
            print_usage(argv[0]);
            return 1;
        }

        return encode(
            argv[2],
            argv[3],
            argv[4],
            argv[5]);
    }

    if (strcmp(argv[1], "decode") == 0)
    {
        if (argc != 5)
        {
            print_usage(argv[0]);
            return 1;
        }

        return decode(
            argv[2],
            argv[3],
            argv[4]);
    }

    if (strcmp(argv[1], "keygen") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return 1;
        }

        return keygen(
            argv[2],
            argv[3]);
    }

    fprintf(stderr,
            "Error: Unknown command '%s'\n\n",
            argv[1]);

    print_usage(argv[0]);
    return 1;
}
