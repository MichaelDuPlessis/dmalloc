#include "bitset.h"

// the number of bits per word
#define BITS_PER_WORD (sizeof(WORD) * 8)

// calculates the number of words for a number of bits
size_t calculate_num_words(size_t num_bits) {
  return (num_bits + BITS_PER_WORD - 1) / BITS_PER_WORD;
}

size_t size_of_bitset(size_t num_bits) {
  return sizeof(BitSet) + calculate_num_words(num_bits);
}

void init_bitset(BitSet *bitset, size_t num_bits) {
  // setting the number of bits
  bitset->num_bits = num_bits;

  // setting the number bits used in the last word of the bitset
  // TODO: I think there is a better way to do this
  bitset->last_word_bits = num_bits % BITS_PER_WORD;

  // getting the number of words
  size_t num_words = calculate_num_words(num_bits);
  // zeroing out buffer
  for (size_t i = 0; i < num_words; i++) {
    bitset->words[i] = 0;
  }
}

void bitset_set(BitSet *bitset, size_t index) {
  if (index >= bitset->num_bits)
    return;

  // figuring out what word contains the bit to flip
  size_t word_idx = index / BITS_PER_WORD;
  // the index within the word that needs to be changed
  size_t bit_idx = index % BITS_PER_WORD;

  // checking if index is in the last word and if there are any unused bits
  if (word_idx == (bitset->num_bits / BITS_PER_WORD) &&
      bitset->last_word_bits != 0) {
    // makign sure that we are not trying to set one of the last invalid bits
    if (bit_idx >= bitset->last_word_bits)
      return;
  }

  bitset->words[word_idx] |= ((size_t)1 << bit_idx);
}
