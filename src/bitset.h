#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <sys/types.h>
#include "allocator.h" // For optimization attributes

// The type to use for a word
#define WORD size_t

// The BitSet struct
typedef struct {
  // Number of bits
  size_t num_bits;
  // The number of bits that are marked
  size_t num_bits_marked;
  // The number number of bits in the last word that are used
  size_t last_word_bits;
  // Keeps track of the last known word with free bits
  size_t free_word_index;
  // The number of words in the bitset
  size_t num_words;
  // The words in the bitset
  WORD words[];
} BitSet;

// Calculates the amount of memory needed for a bitset with num_bits in bytes
DMALLOC_CONST size_t size_of_bitset(size_t num_bits);

// Initializes a bitset from a region of memory. Unused bits are marked (set to 1)
void init_bitset(BitSet *bitset, size_t num_bits);

// Marks the bit (sets it to 1)
DMALLOC_HOT DMALLOC_INLINE void mark_bit(BitSet *bitset, size_t index);

// Clears the bit (sets it to 0)
DMALLOC_HOT DMALLOC_INLINE void unmark_bit(BitSet* bitset, size_t index);

// Flips the bit at the specified location
void flip_bit(BitSet *bitset, size_t index);

// Checks whether the bit is marked or not
// Unused bits will always return true
// If the index is out of range false is returned
DMALLOC_PURE bool check_bit(BitSet *bitset, size_t index);

// Finds the first occurence of an unmarked bit or -1 if none are found
DMALLOC_HOT ssize_t find_first_unmarked_bit(BitSet *bitset);

// Prints the bitset to stdout
DMALLOC_COLD void print_bitset(BitSet *bitset);

// Checks if all bits in the bitset are marked
DMALLOC_PURE bool all_bits_marked(BitSet *bitset);

// Checks if all bits in the bitset are umarked
DMALLOC_PURE bool all_bits_unmarked(BitSet *bitset);

// Clears the bitset marking all bits as unused
void clear_bitset(BitSet *bitset);

#endif
