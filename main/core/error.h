#ifndef KERN_ERROR_H
#define KERN_ERROR_H

#include <stdbool.h>

typedef enum {
    KERN_OK = 0,
    KERN_ERR_INVALID_INPUT,
    KERN_ERR_OUT_OF_MEMORY,
    KERN_ERR_CRYPTO_FAILURE,
    KERN_ERR_QR_PARSE_FAILED,
    KERN_ERR_MNEMONIC_INVALID,
    KERN_ERR_PSBT_INVALID,
    KERN_ERR_NOT_INITIALIZED,
    KERN_ERR_TIMEOUT,
    KERN_ERR_CANCELLED,
    KERN_ERR_IO,
    KERN_ERR_NOT_FOUND,
    KERN_ERR_ALREADY_EXISTS,
    KERN_ERR_BUFFER_TOO_SMALL,
    KERN_ERR_UNSUPPORTED,
} kern_error_t;

const char *kern_error_str(kern_error_t err);

static inline bool kern_is_ok(kern_error_t err) {
    return err == KERN_OK;
}

static inline bool kern_is_error(kern_error_t err) {
    return err != KERN_OK;
}

#endif
