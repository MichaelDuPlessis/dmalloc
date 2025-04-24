#ifndef BITSET_H
#define BITSET_H

#include <stddef.h>

// the type to use for a word
#define WORD size_t

// the BitSet struct
typedef struct {
  // number of bits
  size_t num_bits;
  // the number number of bits in the last word that are used
  size_t last_word_bits;
  // the words in the bitset
  WORD *words;
} BitSet;

// calculates the amount of memory needed for a bitset with num_bits
size_t size_of_bitset(size_t num_bits);

// initializes a bitset from a region of memory
void init_bitset(BitSet *bitset, size_t num_bits);

// sets a bit
void set_bit(BitSet *bitset, size_t index);

// clears the bit
void clear_bit(BitSet* bitset, size_t index);

#endif
