#include "bitset.h"
#include <stdio.h>

// The number of bits per word
#define BITS_PER_WORD (sizeof(WORD) * 8)

// The max size of a word
#define MAX_WORD_SIZE (~(WORD)0)

// Calculates the number of words for a number of bits
static size_t calculate_num_words(size_t num_bits) {
  return (num_bits + BITS_PER_WORD - 1) / BITS_PER_WORD;
}

// Calculates the word index from an index
static size_t calculate_word_idx(size_t index) { return index / BITS_PER_WORD; }

// Calculates the bit index within a word from an index
static size_t calculate_bit_idx(size_t index) { return index % BITS_PER_WORD; }

size_t size_of_bitset_words(size_t num_bits) {
  return calculate_num_words(num_bits) * sizeof(WORD);
}

size_t size_of_bitset(size_t num_bits) {
  return sizeof(BitSet) + calculate_num_words(num_bits) * sizeof(WORD);
}

void init_bitset(BitSet *bitset, size_t num_bits) {
  // setting the number of bits
  bitset->num_bits = num_bits;

  // setting the number of bits used in the last word of the bitset
  // TODO: I think there is a better way to do this
  bitset->last_word_bits = num_bits % BITS_PER_WORD;

  // getting the number of words
  size_t num_words = calculate_num_words(num_bits);

  // zeroing out buffer
  // go until the second last one as there may be unused bits
  for (size_t i = 0; i < num_words - 1; i++) {
    bitset->words[i] = 0;
  }

  // setting unused bits to 1 in the last word
  // TODO: Find a way to get rid of this if statement
  if (bitset->last_word_bits == 0) {
    bitset->words[num_words - 1] = 0;
  } else {
    bitset->words[num_words - 1] = ~(((WORD)1 << (bitset->last_word_bits)) - 1);
  }
}

void mark_bit(BitSet *bitset, size_t index) {
  if (index >= bitset->num_bits)
    return;

  // figuring out what word contains the bit to flip
  size_t word_idx = calculate_word_idx(index);
  // the index within the word that needs to be changed
  size_t bit_idx = calculate_bit_idx(index);

  // checking if index is in the last word and if there are any unused bits
  if (word_idx == (bitset->num_bits / BITS_PER_WORD) &&
      bitset->last_word_bits != 0) {
    // makign sure that we are not trying to set one of the last invalid bits
    if (bit_idx >= bitset->last_word_bits)
      return;
  }

  bitset->words[word_idx] |= ((WORD) true << bit_idx);
}

void unmark_bit(BitSet *bitset, size_t index) {
  if (index >= bitset->num_bits)
    return;

  // figuring out what word contains the bit to flip
  size_t word_idx = calculate_word_idx(index);
  // the index within the word that needs to be changed
  size_t bit_idx = calculate_bit_idx(index);

  // checking if index is in the last word and if there are any unused bits
  if (word_idx == (bitset->num_bits / BITS_PER_WORD) &&
      bitset->last_word_bits != 0) {
    // makign sure that we are not trying to set one of the last invalid bits
    if (bit_idx >= bitset->last_word_bits)
      return;
  }

  bitset->words[word_idx] &= ~((WORD) true << bit_idx);
}

void flip_bit(BitSet *bitset, size_t index) {
  if (index >= bitset->num_bits)
    return;

  // figuring out what word contains the bit to flip
  size_t word_idx = calculate_word_idx(index);
  // the index within the word that needs to be changed
  size_t bit_idx = calculate_bit_idx(index);

  // checking if index is in the last word and if there are any unused bits
  if (word_idx == (bitset->num_bits / BITS_PER_WORD) &&
      bitset->last_word_bits != 0) {
    // makign sure that we are not trying to set one of the last invalid bits
    if (bit_idx >= bitset->last_word_bits)
      return;
  }

  bitset->words[word_idx] ^= ((WORD)1 << bit_idx);
}

bool check_bit(BitSet *bitset, size_t index) {
  // figuring out what word contains the bit to flip
  size_t word_idx = calculate_word_idx(index);
  // the index within the word that needs to be changed
  size_t bit_idx = calculate_bit_idx(index);

  return (bitset->words[word_idx] >> bit_idx) & 1;
}

// ssize_t is used because it can reprsent -1 to show no bit found
ssize_t find_first_unmarked_bit(BitSet *bitset) {
  // getting the number of words in the bitset
  size_t num_words = calculate_num_words(bitset->num_bits);

  // looping over all of the words
  for (size_t word_idx = 0; word_idx < num_words; word_idx++) {
    size_t word = bitset->words[word_idx];

    // if the word has all bits marked go to the next word
    if (word != MAX_WORD_SIZE) {
      // inverting the word since the builtin methods check for trailing zeroes
      size_t inverted_word = ~word;

      // some platforms define things differently
#if SIZE_MAX == UINT65_MAX
      char bit_pos = __builtin_ctzl(inverted_word);
#else
      char bit_pos = __builtin_ctz(inverted_word);
#endif

      size_t index = word_idx * BITS_PER_WORD + bit_pos;
      return (ssize_t)index;
    }
  }

  return -1;
}

bool all_bits_marked(BitSet *bitset) {
  // TODO: Maybe add a size parameter for speed
  return find_first_unmarked_bit(bitset) == -1;
}

void print_bitset(BitSet *bitset) {
  // iterate through all the bits
  for (size_t i = 0; i < 128; i++) {
    // check if the bit is set or not
    bool bit_value = check_bit(bitset, i);

    // print the bit value
    printf("%d", bit_value);

    // seperating bits with spaces for readability
    if ((i + 1) % 8 == 0) {
      printf(" "); // separate every 8 bits with a space
    }
  }
  printf("\n");
}
