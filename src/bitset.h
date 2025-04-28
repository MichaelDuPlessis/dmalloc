#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <sys/types.h>

// The type to use for a word
#define WORD size_t

// The BitSet struct
typedef struct {
  // Number of bits
  size_t num_bits;
  // The number number of bits in the last word that are used
  size_t last_word_bits;
  // The words in the bitset
  // TODO: This currently wastes memory since realistically the array could start here
  // find a way to fix it
  WORD words[];
} BitSet;

// Calculates the amount of memory needed for a bitset with num_bits
size_t size_of_bitset(size_t num_bits);

// Initializes a bitset from a region of memory. Unused bits are marked (set to 1)
void init_bitset(BitSet *bitset, size_t num_bits);

// Marks the bit (sets it to 1)
void mark_bit(BitSet *bitset, size_t index);

// Clears the bit (sets it to 0)
void unmark_bit(BitSet* bitset, size_t index);

// Flips the bit at the specified location
void flip_bit(BitSet *bitset, size_t index);

// Checks whether the bit is marked or not
// Unused bits will always return true
bool check_bit(BitSet *bitset, size_t index);

// Finds the first occurence of an unmarked bit or -1 if none are found
ssize_t find_first_unmarked_bit(BitSet *bitset);

// prints the bitset to stdout
void print_bitset(BitSet *bitset);

#endif
