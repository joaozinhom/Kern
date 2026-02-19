#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

void assert_bool(int actual, int expected, const char* test_name);
void assert_string(const char* actual, const char* expected, const char* test_name);
void assert_int(int actual, int expected, const char* test_name);
void assert_true(int condition, const char* test_name);
void assert_false(int condition, const char* test_name);
#endif