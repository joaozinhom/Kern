// test_utils.c
// Tests for bip39_filter.c, secure_mem.h and memory_utils.h

#include "test.h"
#include "../utils/bip39_filter.h"
#include "../utils/secure_mem.h"
#include "../utils/memory_utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// ─── Helpers ─────────────────────────────────────────────────────────────────

// A known-valid 12-word mnemonic (BIP39 standard test vector)
// All 11 first words are valid BIP39 words; "zoo" is a placeholder for last.
static const char *KNOWN_12_MNEMONIC[12] = {
    "abandon", "abandon", "abandon", "abandon",
    "abandon", "abandon", "abandon", "abandon",
    "abandon", "abandon", "abandon", "about"  // last word valid for this set
};

// A known-valid 24-word mnemonic (BIP39 standard test vector)
static const char *KNOWN_24_MNEMONIC[24] = {
    "abandon", "abandon", "abandon", "abandon",
    "abandon", "abandon", "abandon", "abandon",
    "abandon", "abandon", "abandon", "abandon",
    "abandon", "abandon", "abandon", "abandon",
    "abandon", "abandon", "abandon", "abandon",
    "abandon", "abandon", "abandon", "art"  // last word valid for this set
};

// Helper: copy string list into the fixed 2D array format bip39_filter expects
static void fill_word_array(char out[24][16], const char **words, int count) {
    for (int i = 0; i < count; i++) {
        strncpy(out[i], words[i], 15);
        out[i][15] = '\0';
    }
}

// ─── bip39_filter_init ───────────────────────────────────────────────────────

void test_bip39_filter_init(void) {
    printf("\n-- bip39_filter_init --\n");

    bool result = bip39_filter_init();
    assert_true(result, "init: first call succeeds");

    // Safe to call again (no-op, must not crash or return false)
    result = bip39_filter_init();
    assert_true(result, "init: second call is a no-op and still returns true");
}

// ─── bip39_filter_get_word_index ─────────────────────────────────────────────

void test_bip39_filter_get_word_index(void) {
    printf("\n-- bip39_filter_get_word_index --\n");

    // "abandon" is always index 0 in the English BIP39 wordlist
    assert_int(bip39_filter_get_word_index("abandon"), 0,
               "get_word_index: 'abandon' is index 0");

    // "zoo" is always the last word, index 2047
    assert_int(bip39_filter_get_word_index("zoo"), 2047,
               "get_word_index: 'zoo' is index 2047");

    // Word not in the list
    assert_int(bip39_filter_get_word_index("notaword"), -1,
               "get_word_index: unknown word returns -1");

    // NULL input
    assert_int(bip39_filter_get_word_index(NULL), -1,
               "get_word_index: NULL returns -1");

    // Empty string
    assert_int(bip39_filter_get_word_index(""), -1,
               "get_word_index: empty string returns -1");
}

// ─── bip39_filter_count_matches ──────────────────────────────────────────────

void test_bip39_filter_count_matches(void) {
    printf("\n-- bip39_filter_count_matches --\n");

    // NULL / empty prefix → full wordlist
    assert_int(bip39_filter_count_matches(NULL, 0), BIP39_WORDLIST_SIZE,
               "count_matches: NULL prefix returns full wordlist size");
    assert_int(bip39_filter_count_matches("", 0), BIP39_WORDLIST_SIZE,
               "count_matches: empty prefix returns full wordlist size");

    // "ab" prefix — several words start with "ab" (abandon, ability, able, …)
    int ab_count = bip39_filter_count_matches("ab", 2);
    assert_true(ab_count > 0, "count_matches: 'ab' has matches");
    assert_true(ab_count < BIP39_WORDLIST_SIZE,
                "count_matches: 'ab' is less than full list");

    // Exact full word — exactly 1 match
    assert_int(bip39_filter_count_matches("abandon", 7), 1,
               "count_matches: exact word 'abandon' returns 1");

    // Prefix that matches nothing
    assert_int(bip39_filter_count_matches("zzzzz", 5), 0,
               "count_matches: garbage prefix returns 0");
}

// ─── bip39_filter_by_prefix ──────────────────────────────────────────────────

void test_bip39_filter_by_prefix(void) {
    printf("\n-- bip39_filter_by_prefix --\n");

    const char *out[20];
    int count;

    // Basic match
    count = bip39_filter_by_prefix("ab", 2, out, 20);
    assert_true(count > 0, "by_prefix: 'ab' returns results");
    assert_str_in_array("abandon", out, count,
                        "by_prefix: 'ab' results contain 'abandon'");
    assert_str_in_array("able", out, count,
                        "by_prefix: 'ab' results contain 'able'");

    // max_words cap is respected
    count = bip39_filter_by_prefix("ab", 2, out, 2);
    assert_int(count, 2, "by_prefix: max_words=2 caps results at 2");

    // Exact word
    count = bip39_filter_by_prefix("abandon", 7, out, 20);
    assert_int(count, 1, "by_prefix: exact word returns 1 result");
    assert_string(out[0], "abandon", "by_prefix: exact word result is correct");

    // No match
    count = bip39_filter_by_prefix("zzzzz", 5, out, 20);
    assert_int(count, 0, "by_prefix: no-match prefix returns 0");

    // NULL / edge-case guards
    count = bip39_filter_by_prefix(NULL, 0, out, 20);
    assert_int(count, 0, "by_prefix: NULL prefix returns 0");

    count = bip39_filter_by_prefix("ab", 2, NULL, 20);
    assert_int(count, 0, "by_prefix: NULL out_words returns 0");

    count = bip39_filter_by_prefix("ab", 2, out, 0);
    assert_int(count, 0, "by_prefix: max_words=0 returns 0");
}

// ─── bip39_filter_get_valid_letters ──────────────────────────────────────────

void test_bip39_filter_get_valid_letters(void) {
    printf("\n-- bip39_filter_get_valid_letters --\n");

    uint32_t mask;

    // Empty prefix → all 26 letters must be set (every letter starts some word)
    mask = bip39_filter_get_valid_letters(NULL, 0);
    for (int i = 0; i < 26; i++) {
        char name[64];
        snprintf(name, sizeof(name),
                 "get_valid_letters: empty prefix has bit for '%c'", 'a' + i);
        assert_bit_set(mask, i, name);
    }

    // After "ab", 'a' is valid ('abandon', 'ability', etc.)
    mask = bip39_filter_get_valid_letters("ab", 2);
    assert_bit_set(mask, 'a' - 'a', "get_valid_letters: 'ab'+'a' is valid (abandon…)");

    // After "ab", 'z' should NOT be valid (no bip39 word "abz…")
    assert_bit_clear(mask, 'z' - 'a', "get_valid_letters: 'ab'+'z' is not valid");

    // After a full valid word "abandon", only letters that extend it further
    // should be set — "abandon" has no continuations so mask should be 0
    mask = bip39_filter_get_valid_letters("abandon", 7);
    assert_int((int)mask, 0,
               "get_valid_letters: full word 'abandon' has no next letters");

    // NULL prefix behaves same as empty
    mask = bip39_filter_get_valid_letters(NULL, 0);
    assert_true(mask != 0, "get_valid_letters: NULL prefix returns non-zero mask");
}

// ─── bip39_filter_get_valid_last_words ───────────────────────────────────────

void test_bip39_filter_get_valid_last_words(void) {
    printf("\n-- bip39_filter_get_valid_last_words --\n");

    char words[24][16];
    const char *out[128];
    int count;

    // ── 12-word case ──
    // Fill first 11 words; slot 11 is irrelevant (will be computed)
    fill_word_array(words, KNOWN_12_MNEMONIC, 11);

    bip39_filter_clear_last_word_cache();
    count = bip39_filter_get_valid_last_words(words, 12, out, 128);

    // For 12 words there are 2^7 = 128 possible last-word entropy values,
    // each produces a unique valid word → expect exactly 128 results
    assert_int(count, 128, "get_valid_last_words: 12-word mnemonic yields 128 candidates");

    // The known correct last word for all-"abandon" 11 words is "about"
    assert_str_in_array("about", out, count,
                        "get_valid_last_words: 12-word known last word 'about' present");

    // ── 24-word case ──
    fill_word_array(words, KNOWN_24_MNEMONIC, 23);

    bip39_filter_clear_last_word_cache();
    count = bip39_filter_get_valid_last_words(words, 24, out, 128);

    // For 24 words there are 2^3 = 8 possible last words
    assert_int(count, 8, "get_valid_last_words: 24-word mnemonic yields 8 candidates");

    assert_str_in_array("art", out, count,
                        "get_valid_last_words: 24-word known last word 'art' present");

    // ── Guard: invalid word_count ──
    bip39_filter_clear_last_word_cache();
    count = bip39_filter_get_valid_last_words(words, 13, out, 128);
    assert_int(count, 0, "get_valid_last_words: word_count=13 returns 0");

    // ── Guard: unknown word in list ──
    fill_word_array(words, KNOWN_12_MNEMONIC, 11);
    strncpy(words[0], "notaword", 15);
    bip39_filter_clear_last_word_cache();
    count = bip39_filter_get_valid_last_words(words, 12, out, 128);
    assert_int(count, 0, "get_valid_last_words: invalid word in list returns 0");

    // ── Guard: NULL inputs ──
    bip39_filter_clear_last_word_cache();
    count = bip39_filter_get_valid_last_words(NULL, 12, out, 128);
    assert_int(count, 0, "get_valid_last_words: NULL entered_words returns 0");

    count = bip39_filter_get_valid_last_words(words, 12, NULL, 128);
    assert_int(count, 0, "get_valid_last_words: NULL out_words returns 0");

    count = bip39_filter_get_valid_last_words(words, 12, out, 0);
    assert_int(count, 0, "get_valid_last_words: max_words=0 returns 0");
}

// ─── bip39_filter_get_valid_letters_for_last_word ────────────────────────────

void test_bip39_filter_get_valid_letters_for_last_word(void) {
    printf("\n-- bip39_filter_get_valid_letters_for_last_word --\n");

    char words[24][16];
    uint32_t mask;

    fill_word_array(words, KNOWN_12_MNEMONIC, 11);
    bip39_filter_clear_last_word_cache();

    // No prefix: should allow at least 'a' (since "about" is a valid last word)
    mask = bip39_filter_get_valid_letters_for_last_word(words, 12, NULL, 0);
    assert_true(mask != 0,
                "get_valid_letters_last_word: no prefix returns non-zero mask");
    assert_bit_set(mask, 'a' - 'a',
                   "get_valid_letters_last_word: 'a' is set (leads to 'about')");

    // With prefix "ab": 'o' should be valid (about), 'z' should not
    mask = bip39_filter_get_valid_letters_for_last_word(words, 12, "ab", 2);
    assert_bit_set(mask, 'o' - 'a',
                   "get_valid_letters_last_word: prefix 'ab'+'o' valid (about)");
    assert_bit_clear(mask, 'z' - 'a',
                     "get_valid_letters_last_word: prefix 'ab'+'z' not valid");

    // Invalid word_count: should return 0
    bip39_filter_clear_last_word_cache();
    mask = bip39_filter_get_valid_letters_for_last_word(words, 5, NULL, 0);
    assert_int((int)mask, 0,
               "get_valid_letters_last_word: invalid word_count returns 0");
}

// ─── bip39_filter_last_word_by_prefix ────────────────────────────────────────

void test_bip39_filter_last_word_by_prefix(void) {
    printf("\n-- bip39_filter_last_word_by_prefix --\n");

    char words[24][16];
    const char *out[128];
    int count;

    fill_word_array(words, KNOWN_12_MNEMONIC, 11);
    bip39_filter_clear_last_word_cache();

    // No prefix → all 128 valid last words returned
    count = bip39_filter_last_word_by_prefix(words, 12, NULL, 0, out, 128);
    assert_int(count, 128,
               "last_word_by_prefix: no prefix returns all 128 candidates");

    // Prefix "ab" → should contain "about"
    bip39_filter_clear_last_word_cache();
    count = bip39_filter_last_word_by_prefix(words, 12, "ab", 2, out, 128);
    assert_true(count > 0, "last_word_by_prefix: prefix 'ab' has results");
    assert_str_in_array("about", out, count,
                        "last_word_by_prefix: 'about' found with prefix 'ab'");

    // max_words cap is respected
    bip39_filter_clear_last_word_cache();
    count = bip39_filter_last_word_by_prefix(words, 12, NULL, 0, out, 3);
    assert_int(count, 3, "last_word_by_prefix: max_words=3 caps at 3");

    // Prefix that matches no valid last word
    bip39_filter_clear_last_word_cache();
    count = bip39_filter_last_word_by_prefix(words, 12, "zzz", 3, out, 128);
    assert_int(count, 0, "last_word_by_prefix: no-match prefix returns 0");

    // Guards
    count = bip39_filter_last_word_by_prefix(words, 12, "ab", 2, NULL, 128);
    assert_int(count, 0, "last_word_by_prefix: NULL out_words returns 0");

    count = bip39_filter_last_word_by_prefix(words, 12, "ab", 2, out, 0);
    assert_int(count, 0, "last_word_by_prefix: max_words=0 returns 0");
}

// ─── bip39_filter_clear_last_word_cache ──────────────────────────────────────

void test_bip39_filter_clear_last_word_cache(void) {
    printf("\n-- bip39_filter_clear_last_word_cache --\n");

    char words[24][16];
    const char *out[128];

    fill_word_array(words, KNOWN_12_MNEMONIC, 11);

    // Populate cache
    bip39_filter_clear_last_word_cache();
    bip39_filter_get_valid_last_words(words, 12, out, 128);

    // After clearing, last_word_by_prefix with a fresh word set should
    // recalculate rather than return stale data.
    // We verify by clearing and then calling again — must not crash and
    // must return a consistent result.
    bip39_filter_clear_last_word_cache();
    int count = bip39_filter_get_valid_last_words(words, 12, out, 128);
    assert_int(count, 128,
               "clear_cache: after clear, recalculation still returns 128");

    // Calling clear multiple times must not crash
    bip39_filter_clear_last_word_cache();
    bip39_filter_clear_last_word_cache();
    assert_true(1, "clear_cache: multiple clears do not crash");
}

// ─── secure_memzero ───────────────────────────────────────────────────────────

void test_secure_memzero(void) {
    printf("\n-- secure_memzero --\n");

    unsigned char buf[32];
    memset(buf, 0xAB, sizeof(buf));

    secure_memzero(buf, sizeof(buf));
    assert_mem_zero(buf, sizeof(buf), "secure_memzero: 32-byte buffer is zeroed");

    // Partial zero
    memset(buf, 0xFF, sizeof(buf));
    secure_memzero(buf, 8);
    assert_mem_zero(buf, 8, "secure_memzero: first 8 bytes are zeroed");
    // The rest must be untouched
    assert_true(buf[8] == 0xFF,
                "secure_memzero: byte beyond range is not zeroed");

    // NULL and zero-length must not crash
    secure_memzero(NULL, 10);
    assert_true(1, "secure_memzero: NULL pointer does not crash");

    secure_memzero(buf, 0);
    assert_true(1, "secure_memzero: zero length does not crash");
}

// ─── secure_memcmp ────────────────────────────────────────────────────────────

void test_secure_memcmp(void) {
    printf("\n-- secure_memcmp --\n");

    unsigned char a[] = {0x01, 0x02, 0x03, 0x04};
    unsigned char b[] = {0x01, 0x02, 0x03, 0x04};
    unsigned char c[] = {0x01, 0x02, 0x03, 0xFF};

    assert_int(secure_memcmp(a, b, 4), 0,
               "secure_memcmp: identical buffers return 0");

    assert_true(secure_memcmp(a, c, 4) != 0,
                "secure_memcmp: different buffers return non-zero");

    // Single byte difference
    assert_true(secure_memcmp(a, c, 3) == 0,
                "secure_memcmp: first 3 bytes equal returns 0");
    assert_true(secure_memcmp(a, c, 4) != 0,
                "secure_memcmp: 4 bytes with last different returns non-zero");

    // Zero length
    assert_int(secure_memcmp(a, c, 0), 0,
               "secure_memcmp: zero length always returns 0");

    // Compare against itself
    assert_int(secure_memcmp(a, a, 4), 0,
               "secure_memcmp: same pointer returns 0");
}

// ─── SAFE_FREE_STATIC (memory_utils.h) ───────────────────────────────────────

void test_safe_free_static(void) {
    printf("\n-- SAFE_FREE_STATIC --\n");

    char *ptr = (char *)malloc(16);
    assert_true(ptr != NULL, "SAFE_FREE_STATIC: allocation succeeds");

    SAFE_FREE_STATIC(ptr);
    assert_true(ptr == NULL, "SAFE_FREE_STATIC: pointer is NULL after free");

    // Calling again on NULL must not crash
    SAFE_FREE_STATIC(ptr);
    assert_true(1, "SAFE_FREE_STATIC: double-free on NULL does not crash");
}

// ─── SAFE_STRDUP (memory_utils.h) ────────────────────────────────────────────

void test_safe_strdup(void) {
    printf("\n-- SAFE_STRDUP --\n");

    char *dup = SAFE_STRDUP("hello");
    assert_true(dup != NULL, "SAFE_STRDUP: returns non-NULL for valid string");
    assert_string(dup, "hello", "SAFE_STRDUP: duplicated content matches");
    free(dup);

    char *null_dup = SAFE_STRDUP(NULL);
    assert_true(null_dup == NULL, "SAFE_STRDUP: NULL input returns NULL");

    char *empty = SAFE_STRDUP("");
    assert_true(empty != NULL, "SAFE_STRDUP: empty string returns non-NULL");
    assert_string(empty, "", "SAFE_STRDUP: empty string content matches");
    free(empty);
}

// ─── SECURE_FREE_STRING (secure_mem.h) ───────────────────────────────────────

void test_secure_free_string(void) {
    printf("\n-- SECURE_FREE_STRING --\n");

    char *s = strdup("secret_data");
    assert_true(s != NULL, "SECURE_FREE_STRING: allocation succeeds");

    SECURE_FREE_STRING(s);
    assert_true(s == NULL, "SECURE_FREE_STRING: pointer is NULL after macro");

    // Calling again on NULL must not crash
    SECURE_FREE_STRING(s);
    assert_true(1, "SECURE_FREE_STRING: double-call on NULL does not crash");
}

// ─── SECURE_FREE_BUFFER (secure_mem.h) ───────────────────────────────────────

void test_secure_free_buffer(void) {
    printf("\n-- SECURE_FREE_BUFFER --\n");

    size_t len = 64;
    uint8_t *buf = (uint8_t *)malloc(len);
    assert_true(buf != NULL, "SECURE_FREE_BUFFER: allocation succeeds");
    memset(buf, 0xCC, len);

    SECURE_FREE_BUFFER(buf, len);
    assert_true(buf == NULL, "SECURE_FREE_BUFFER: pointer is NULL after macro");

    // NULL safe
    SECURE_FREE_BUFFER(buf, len);
    assert_true(1, "SECURE_FREE_BUFFER: double-call on NULL does not crash");
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main(void) {
    printf("=== test_utils ===\n");

    // Init must be first — all bip39 tests depend on it
    test_bip39_filter_init();

    test_bip39_filter_get_word_index();
    test_bip39_filter_count_matches();
    test_bip39_filter_by_prefix();
    test_bip39_filter_get_valid_letters();
    test_bip39_filter_get_valid_last_words();
    test_bip39_filter_get_valid_letters_for_last_word();
    test_bip39_filter_last_word_by_prefix();
    test_bip39_filter_clear_last_word_cache();

    test_secure_memzero();
    test_secure_memcmp();

    test_safe_free_static();
    test_safe_strdup();
    test_secure_free_string();
    test_secure_free_buffer();

    printf("\n=== done ===\n");
    return 0;
}