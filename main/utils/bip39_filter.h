// BIP39 word filtering utilities for smart keyboard input

#ifndef BIP39_FILTER_H
#define BIP39_FILTER_H

#include <stdbool.h>
#include <stdint.h>

#define BIP39_WORDLIST_SIZE 2048
#define BIP39_MAX_FILTERED_WORDS 8
#define BIP39_MAX_PREFIX_LEN 8

/**
 * Initialize the BIP39 wordlist. Must be called before other functions.
 * Safe to call multiple times (subsequent calls are no-ops).
 * @return true on success, false on failure
 */
bool bip39_filter_init(void);

/**
 * Get a bitmask of valid next letters for a given prefix.
 * Bit N is set if appending letter ('a' + N) would match at least one word.
 * @param prefix Current prefix string
 * @param prefix_len Length of prefix
 * @return 26-bit mask (bits 0-25 for a-z), or 0xFFFFFFFF if wordlist not loaded
 */
uint32_t bip39_filter_get_valid_letters(const char *prefix, int prefix_len);

/**
 * Filter words by prefix and return matches.
 * @param prefix Current prefix string
 * @param prefix_len Length of prefix
 * @param out_words Array to receive matching word pointers (points to wordlist)
 * @param max_words Maximum number of words to return
 * @return Number of words written to out_words
 */
int bip39_filter_by_prefix(const char *prefix, int prefix_len,
                           const char **out_words, int max_words);

/**
 * Count total number of words matching a prefix.
 * @param prefix Current prefix string
 * @param prefix_len Length of prefix
 * @return Number of matching words, or BIP39_WORDLIST_SIZE if prefix is empty
 */
int bip39_filter_count_matches(const char *prefix, int prefix_len);

/**
 * Get the index (0-2047) of a BIP39 word.
 * @param word The word to look up
 * @return Word index (0-2047), or -1 if not found
 */
int bip39_filter_get_word_index(const char *word);

#endif // BIP39_FILTER_H
