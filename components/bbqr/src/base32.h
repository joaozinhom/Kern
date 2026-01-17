#ifndef BASE32_H
#define BASE32_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Calculate the encoded length for base32 encoding
 *
 * @param input_len Length of input data in bytes
 * @return Size of base32 encoded output (without null terminator)
 */
size_t base32_encoded_len(size_t input_len);

/**
 * @brief Calculate the maximum decoded length for base32 decoding
 *
 * @param input_len Length of base32 encoded string (excluding padding)
 * @return Maximum size of decoded output in bytes
 */
size_t base32_decoded_len(size_t input_len);

/**
 * @brief Encode binary data to base32 (RFC 4648)
 *
 * Uses the standard base32 alphabet: ABCDEFGHIJKLMNOPQRSTUVWXYZ234567
 * Output is uppercase and includes padding with '=' characters.
 *
 * @param input Binary data to encode
 * @param input_len Length of input data in bytes
 * @param output Buffer to store base32 encoded string (must be at least
 * base32_encoded_len(input_len) + 1 bytes)
 * @param output_size Size of output buffer
 * @return Length of encoded string on success, 0 on failure
 */
size_t base32_encode(const uint8_t *input, size_t input_len, char *output,
                     size_t output_size);

/**
 * @brief Decode base32 string to binary data (RFC 4648)
 *
 * Accepts both uppercase and lowercase input.
 * Handles padding characters ('=') and ignores them.
 *
 * @param input Base32 encoded string
 * @param input_len Length of input string
 * @param output Buffer to store decoded binary data (must be at least
 * base32_decoded_len(input_len) bytes)
 * @param output_size Size of output buffer
 * @param out_len Pointer to store actual decoded length
 * @return true on success, false on failure (invalid input)
 */
bool base32_decode(const char *input, size_t input_len, uint8_t *output,
                   size_t output_size, size_t *out_len);

#endif /* BASE32_H */
