#include"tests.h"

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