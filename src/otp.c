
#include "otp.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/*
 * XOR encryption and decryption.
 *
 * Ciphertext = Plaintext XOR Key
 * Plaintext  = Ciphertext XOR Key
 */
int otp_xor(const uint8_t *input,
            const uint8_t *key,
            uint8_t *output,
            size_t length)
{
    if (!input || !key || !output || length == 0)
    {
        fprintf(stderr, "Error: Invalid OTP input\n");
        return -1;
    }

    for (size_t i = 0; i < length; i++)
    {
        output[i] = input[i] ^ key[i];
    }

    return 0;
}

/*
 * Generate a cryptographically random OTP key.
 *
 * /dev/urandom provides random bytes
 */
int otp_generate_key(uint8_t *key, size_t length)
{
    if (!key || length == 0)
    {
        fprintf(stderr, "Error: Invalid key buffer\n");
        return -1;
    }

    FILE *fp = fopen("/dev/urandom", "rb");

    if (!fp)
    {
        perror("Error opening /dev/urandom");
        return -1;
    }

    size_t bytes_read = fread(key, 1, length, fp);

    fclose(fp);

    if (bytes_read != length)
    {
        fprintf(stderr, "Error: Failed to generate OTP key\n");
        return -1;
    }

    return 0;
}
