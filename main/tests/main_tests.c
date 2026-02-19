#include"tests.h"

int main() {
    assert_bool(1, 1, "Test true is true");
    assert_bool(0, 1, "Test false is true");
    assert_string("cat", "cat", "Test string equals cat");
    assert_string("dog", "cat", "Test string equals cat");
    assert_int(42, 42, "Test int equals 42");
    assert_true(1 == 1, "Test 1 equals 1");
    assert_false(1 == 0, "Test 1 not equals 0");
    return 0;
}