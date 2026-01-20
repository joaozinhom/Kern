// BIP39 word filtering utilities for smart keyboard input

#include "bip39_filter.h"
#include <string.h>
#include <wally_bip39.h>

static struct words *wordlist = NULL;

bool bip39_filter_init(void) {
  if (wordlist)
    return true;
  return bip39_get_wordlist(NULL, &wordlist) == WALLY_OK;
}

uint32_t bip39_filter_get_valid_letters(const char *prefix, int prefix_len) {
  uint32_t mask = 0;
  if (!wordlist)
    return 0xFFFFFFFF;

  for (int letter = 0; letter < 26; letter++) {
    char test_prefix[BIP39_MAX_PREFIX_LEN + 2];
    int test_len = prefix_len + 1;

    if (prefix && prefix_len > 0) {
      int copy_len = prefix_len < BIP39_MAX_PREFIX_LEN ? prefix_len : BIP39_MAX_PREFIX_LEN;
      memcpy(test_prefix, prefix, copy_len);
      test_prefix[copy_len] = 'a' + letter;
      test_prefix[copy_len + 1] = '\0';
    } else {
      test_prefix[0] = 'a' + letter;
      test_prefix[1] = '\0';
      test_len = 1;
    }

    for (size_t i = 0; i < BIP39_WORDLIST_SIZE; i++) {
      const char *word = bip39_get_word_by_index(wordlist, i);
      if (word && strncmp(word, test_prefix, test_len) == 0) {
        mask |= (1u << letter);
        break;
      }
    }
  }
  return mask;
}

int bip39_filter_by_prefix(const char *prefix, int prefix_len,
                           const char **out_words, int max_words) {
  int count = 0;
  if (!wordlist || !out_words || max_words <= 0)
    return 0;
  if (!prefix || prefix_len <= 0)
    return 0;

  for (size_t i = 0; i < BIP39_WORDLIST_SIZE && count < max_words; i++) {
    const char *word = bip39_get_word_by_index(wordlist, i);
    if (word && strncmp(word, prefix, prefix_len) == 0) {
      out_words[count++] = word;
    }
  }
  return count;
}

int bip39_filter_count_matches(const char *prefix, int prefix_len) {
  int count = 0;
  if (!wordlist)
    return 0;
  if (!prefix || prefix_len <= 0)
    return BIP39_WORDLIST_SIZE;

  for (size_t i = 0; i < BIP39_WORDLIST_SIZE; i++) {
    const char *word = bip39_get_word_by_index(wordlist, i);
    if (word && strncmp(word, prefix, prefix_len) == 0)
      count++;
  }
  return count;
}

int bip39_filter_get_word_index(const char *word) {
  if (!wordlist || !word)
    return -1;

  for (size_t i = 0; i < BIP39_WORDLIST_SIZE; i++) {
    const char *w = bip39_get_word_by_index(wordlist, i);
    if (w && strcmp(w, word) == 0)
      return (int)i;
  }
  return -1;
}
