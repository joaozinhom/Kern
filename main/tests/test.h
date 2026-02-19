#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>
#include <wally_bip39.h>
#include "../utils/bip39_filter.h"
#include "../utils/secure_mem.h"
#include "../utils/memory_utils.h"
#include <stdlib.h> 


typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;


void assert_bool(int actual, int expected, const char* test_name);
void assert_string(const char* actual, const char* expected, const char* test_name);
void assert_int(int actual, int expected, const char* test_name);
void assert_true(int condition, const char* test_name);
void assert_false(int condition, const char* test_name);
// For bitmask testing
void assert_bit_set(uint32_t mask, int bit, const char* test_name);
void assert_bit_clear(uint32_t mask, int bit, const char* test_name);

// For buffer/memory testing  
void assert_mem_zero(const void* buf, size_t len, const char* test_name);

// For array/set membership
void assert_str_in_array(const char* needle, const char** arr, int count, const char* test_name);
#endif
