
#ifndef OTP_H
#define OTP_H

#include <stddef.h>
#include <stdint.h>

/*
 * Encrypt or decrypt data using XOR.
 *
 * input:  Plaintext or ciphertext
 * key:    OTP key bytes
 * output: Buffer receiving result
 * length: Number of bytes to process
 *
 * The key and output buffers must contain
 * at least length bytes.
 *
 * Returns 0 on success, -1 on failure.
 */
int otp_xor(const uint8_t *input,
            const uint8_t *key,
            uint8_t *output,
            size_t length);

/*
 * Generate a random OTP key.
 *
 * Uses the operating system's secure
 * random number generator.
 *
 * Returns 0 on success, -1 on failure.
 */
int otp_generate_key(uint8_t *key, size_t length);

#endif
