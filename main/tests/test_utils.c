#include "tests.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for utils.c functions
bool str_has_prefix(const char *str, const char *prefix);
void str_to_lower(char *str);
size_t str_split(const char *str, char delimiter, char **parts, size_t max_parts);
bool is_ur_type(const char *type);
bool parse_ur_string(const char *ur_str, char **type, char ***components, size_t *component_count);
bool parse_sequence_component(const char *seq_str, uint32_t *seq_num, size_t *seq_len);
void free_string_array(char **strings, size_t count);
void *safe_malloc(size_t size);
char *safe_strdup(const char *str);

void test_str_to_lower() {
    char s1[] = "ABC";
    str_to_lower(s1);
    assert_string(s1, "abc", "str_to_lower: 'ABC' -> 'abc'");

    char s2[] = "aBcDeF";
    str_to_lower(s2);
    assert_string(s2, "abcdef", "str_to_lower: 'aBcDeF' -> 'abcdef'");

    char s3[] = "123!@#";
    str_to_lower(s3);
    assert_string(s3, "123!@#", "str_to_lower: '123!@#' unchanged");

    str_to_lower(NULL); // Should not crash
    assert_true(1, "str_to_lower: NULL input does not crash");
}

void test_str_split() {
    char *parts[5];
    size_t count = str_split("a,b,c", ',', parts, 5);
    assert_int(count, 3, "str_split: split 'a,b,c'");
    assert_string(parts[0], "a", "str_split: part 0 is 'a'");
    assert_string(parts[1], "b", "str_split: part 1 is 'b'");
    assert_string(parts[2], "c", "str_split: part 2 is 'c'");
    free_string_array(parts, count);

    count = str_split("a,,b", ',', parts, 5);
    assert_int(count, 2, "str_split: split 'a,,b' (empty part skipped)");
    assert_string(parts[0], "a", "str_split: part 0 is 'a'");
    assert_string(parts[1], "b", "str_split: part 1 is 'b'");
    free_string_array(parts, count);

    count = str_split("", ',', parts, 5);
    assert_int(count, 0, "str_split: empty string");

    count = str_split(NULL, ',', parts, 5);
    assert_int(count, 0, "str_split: NULL string");

    count = str_split("a,b,c", ',', NULL, 5);
    assert_int(count, 0, "str_split: NULL parts array");

    count = str_split("a,b,c", ',', parts, 0);
    assert_int(count, 0, "str_split: max_parts = 0");
}

void test_is_ur_type() {
    assert_true(is_ur_type("crypto-seed"), "is_ur_type: valid type");
    assert_true(is_ur_type("abc-123"), "is_ur_type: valid type with digits");
    assert_false(is_ur_type("Crypto-seed"), "is_ur_type: uppercase letter");
    assert_false(is_ur_type("-abc"), "is_ur_type: starts with '-'");
    assert_false(is_ur_type("abc-"), "is_ur_type: ends with '-'");
    assert_false(is_ur_type("abc@123"), "is_ur_type: invalid char '@'");
    assert_false(is_ur_type(""), "is_ur_type: empty string");
    assert_false(is_ur_type(NULL), "is_ur_type: NULL input");
}

void test_parse_ur_string() {
    char *type = NULL;
    char **components = NULL;
    size_t component_count = 0;

    bool result = parse_ur_string("ur:crypto-seed/abcd/efgh", &type, &components, &component_count);
    assert_true(result, "parse_ur_string: valid UR");
    assert_string(type, "crypto-seed", "parse_ur_string: type");
    assert_int(component_count, 2, "parse_ur_string: component count");
    assert_string(components[0], "abcd", "parse_ur_string: component 0");
    assert_string(components[1], "efgh", "parse_ur_string: component 1");
    free(type);
    free_string_array(components, component_count);
    free(components);

    result = parse_ur_string("ur:invalid-/abcd", &type, &components, &component_count);
    assert_false(result, "parse_ur_string: invalid type");

    result = parse_ur_string("ur:", &type, &components, &component_count);
    assert_false(result, "parse_ur_string: empty path");

    result = parse_ur_string(NULL, &type, &components, &component_count);
    assert_false(result, "parse_ur_string: NULL input");

    result = parse_ur_string("ur:crypto-seed", &type, &components, &component_count);
    assert_false(result, "parse_ur_string: only type, no components");

    result = parse_ur_string("ur:crypto-seed/", &type, &components, &component_count);
    assert_false(result, "parse_ur_string: trailing slash, no component");
}

void test_parse_sequence_component() {
    uint32_t seq_num;
    size_t seq_len;

    bool result = parse_sequence_component("1-10", &seq_num, &seq_len);
    assert_true(result, "parse_sequence_component: valid");
    assert_int(seq_num, 1, "parse_sequence_component: seq_num");
    assert_int(seq_len, 10, "parse_sequence_component: seq_len");

    result = parse_sequence_component("0-10", &seq_num, &seq_len);
    assert_false(result, "parse_sequence_component: seq_num zero");

    result = parse_sequence_component("1-0", &seq_num, &seq_len);
    assert_false(result, "parse_sequence_component: seq_len zero");

    result = parse_sequence_component("1-ten", &seq_num, &seq_len);
    assert_false(result, "parse_sequence_component: non-numeric seq_len");

    result = parse_sequence_component("ten-1", &seq_num, &seq_len);
    assert_false(result, "parse_sequence_component: non-numeric seq_num");

    result = parse_sequence_component(NULL, &seq_num, &seq_len);
    assert_false(result, "parse_sequence_component: NULL input");

    result = parse_sequence_component("1", &seq_num, &seq_len);
    assert_false(result, "parse_sequence_component: missing '-'");
}

void test_safe_malloc_and_strdup() {
    char *mem = safe_malloc(10);
    assert_true(mem != NULL, "safe_malloc: allocates memory");
    memset(mem, 'a', 9);
    mem[9] = '\0';
    assert_string(mem, "aaaaaaaaa", "safe_malloc: memory writable");
    free(mem);

    char *dup = safe_strdup("test");
    assert_true(dup != NULL, "safe_strdup: duplicates string");
    assert_string(dup, "test", "safe_strdup: duplicated string matches");
    free(dup);

    assert_true(safe_malloc(0) == NULL, "safe_malloc: zero size returns NULL");
    assert_true(safe_strdup(NULL) == NULL, "safe_strdup: NULL input returns NULL");
}

int main() {
    test_str_to_lower();
    test_str_split();
    test_is_ur_type();
    test_parse_ur_string();
    test_parse_sequence_component();
    test_safe_malloc_and_strdup();
    return 0;
}