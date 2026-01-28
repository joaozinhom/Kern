#include "error.h"

const char *kern_error_str(kern_error_t err) {
    switch (err) {
        case KERN_OK:
            return "Success";
        case KERN_ERR_INVALID_INPUT:
            return "Invalid input";
        case KERN_ERR_OUT_OF_MEMORY:
            return "Out of memory";
        case KERN_ERR_CRYPTO_FAILURE:
            return "Cryptographic error";
        case KERN_ERR_QR_PARSE_FAILED:
            return "QR parse failed";
        case KERN_ERR_MNEMONIC_INVALID:
            return "Invalid mnemonic";
        case KERN_ERR_PSBT_INVALID:
            return "Invalid PSBT";
        case KERN_ERR_NOT_INITIALIZED:
            return "Not initialized";
        case KERN_ERR_TIMEOUT:
            return "Timeout";
        case KERN_ERR_CANCELLED:
            return "Cancelled";
        case KERN_ERR_IO:
            return "I/O error";
        case KERN_ERR_NOT_FOUND:
            return "Not found";
        case KERN_ERR_ALREADY_EXISTS:
            return "Already exists";
        case KERN_ERR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case KERN_ERR_UNSUPPORTED:
            return "Not supported";
        default:
            return "Unknown error";
    }
}
