#include "test.h"

void assert_bool(int actual, int expected, const char* test_name) {
    if (actual == expected) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected %d, but got %d\n", test_name, expected, actual);
    }
}

void assert_string(const char* actual, const char* expected, const char* test_name) {
    if (strcmp(actual, expected) == 0) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected '%s', but got '%s'\n", test_name, expected, actual);
    }
}

void assert_int(int actual, int expected, const char* test_name) {
    if (actual == expected) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected %d, but got %d\n", test_name, expected, actual);
    }
}

void assert_true(int condition, const char* test_name) {
    if (condition) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected true but got false\n", test_name);
    }
}

void assert_false(int condition, const char* test_name) {
    if (!condition) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected false but got true\n", test_name);
    }
}


void assert_bit_set(uint32_t mask, int bit, const char* test_name) {
    if (bit < 0 || bit > 31) {
        printf("[FAIL] %s: Bit %d is out of range (0-31)\n", test_name, bit);
        return;
    }
    if (mask & (1u << bit)) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected bit %d to be SET in mask 0x%08X\n",
               test_name, bit, mask);
    }
}

void assert_bit_clear(uint32_t mask, int bit, const char* test_name) {
    if (bit < 0 || bit > 31) {
        printf("[FAIL] %s: Bit %d is out of range (0-31)\n", test_name, bit);
        return;
    }
    if (!(mask & (1u << bit))) {
        printf("[PASS] %s\n", test_name);
    } else {
        printf("[FAIL] %s: Expected bit %d to be CLEAR in mask 0x%08X\n",
               test_name, bit, mask);
    }
}


void assert_mem_zero(const void* buf, size_t len, const char* test_name) {
    if (!buf) {
        printf("[FAIL] %s: Buffer is NULL\n", test_name);
        return;
    }
    const unsigned char* p = (const unsigned char*)buf;
    for (size_t i = 0; i < len; i++) {
        if (p[i] != 0) {
            printf("[FAIL] %s: Byte at index %zu is 0x%02X, expected 0x00\n",
                   test_name, i, p[i]);
            return;
        }
    }
    printf("[PASS] %s\n", test_name);
}


void assert_str_in_array(const char* needle, const char** arr, int count,
                         const char* test_name) {
    if (!needle || !arr) {
        printf("[FAIL] %s: NULL needle or array\n", test_name);
        return;
    }
    for (int i = 0; i < count; i++) {
        if (arr[i] && strcmp(arr[i], needle) == 0) {
            printf("[PASS] %s\n", test_name);
            return;
        }
    }
    printf("[FAIL] %s: '%s' not found in array of %d elements\n",
           test_name, needle, count);
}