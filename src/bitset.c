#include "bitset.h"
#include <stdio.h>
#include <string.h>

// The number of bits per word
#define BITS_PER_WORD (sizeof(WORD) * 8)
// Sadly there is no way to calculate this at compile time
#define LOG2_BITS_PER_WORD (6)

// The max size of a word
#define MAX_WORD_SIZE (~(WORD)0)

// Calculates the number of words for a number of bits
static inline size_t calculate_num_words(size_t num_bits) {
  return (num_bits + BITS_PER_WORD - 1) / BITS_PER_WORD;
}

// masks the unused bits
static inline WORD unused_bit_mask(size_t bits) {
  return ~(((WORD)1 << bits) - 1);
}

// Calculates the word index from an index
static inline size_t calculate_word_idx(size_t index) {
  // this works because dividing by 64 (the number of bits in a word)
  // is equivelant to dividing by 2 6 times an dividing by 2 can be done
  // using bit shifts which are generally faster
  return index >> LOG2_BITS_PER_WORD;
}

// Calculates the bit index within a word from an index
static inline size_t calculate_bit_idx(size_t index) {
  return index & (BITS_PER_WORD - 1);
}

static inline size_t size_of_bitset_words(size_t num_bits) {
  return calculate_num_words(num_bits) * sizeof(WORD);
}

size_t size_of_bitset(size_t num_bits) {
  return sizeof(BitSet) + size_of_bitset_words(num_bits);
}

void init_bitset(BitSet *bitset, size_t num_bits) {
  // setting the number of bits
  bitset->num_bits = num_bits;

  // setting the number of words
  bitset->num_words = calculate_num_words(num_bits);

  // setting the number of marked bits
  bitset->num_bits_marked = 0;

  // since the bitset was just initialized the index of a word
  // with free bits is the first one
  bitset->free_word_index = 0;

  // setting the number of bits used in the last word of the bitset
  bitset->last_word_bits = num_bits % BITS_PER_WORD;

  clear_bitset(bitset);
}

void mark_bit(BitSet *bitset, size_t index) {
  // Fast bounds check with branch prediction hint
  if (__builtin_expect(index >= bitset->num_bits, 0))
    return;

  // Calculate word and bit indices
  size_t word_idx = calculate_word_idx(index);
  size_t bit_idx = calculate_bit_idx(index);

  // Create bitmask for the target bit
  WORD bitmask = (WORD)1 << bit_idx;
  
  // Check if bit is already marked (avoid unnecessary updates)
  if (__builtin_expect((bitset->words[word_idx] & bitmask) == 0, 1)) {
    // Update the word and increment marked bit count
    bitset->words[word_idx] |= bitmask;
    bitset->num_bits_marked++;
    
    // Update free_word_index if this word becomes full
    if (__builtin_expect(bitset->words[word_idx] == MAX_WORD_SIZE && word_idx == bitset->free_word_index, 0)) {
      // Find next word with free bits
      size_t next_free = word_idx + 1;
      while (next_free < bitset->num_words && bitset->words[next_free] == MAX_WORD_SIZE) {
        next_free++;
      }
      bitset->free_word_index = next_free;
    }
  }
}

void unmark_bit(BitSet *bitset, size_t index) {
  // Fast bounds check
  if (__builtin_expect(index >= bitset->num_bits, 0))
    return;

  // Calculate word and bit indices
  size_t word_idx = calculate_word_idx(index);
  size_t bit_idx = calculate_bit_idx(index);

  // Fast path: check if we're trying to unmark an unused bit in the last word
  if (__builtin_expect(word_idx == (bitset->num_words - 1) && 
                      bitset->last_word_bits != 0 && 
                      bit_idx >= bitset->last_word_bits, 0)) {
    return;
  }

  // Create bitmask for the target bit
  WORD bitmask = (WORD)1 << bit_idx;
  
  // Check if bit is marked before unmarking
  if (__builtin_expect((bitset->words[word_idx] & bitmask) != 0, 1)) {
    // Update the word and decrement marked bit count
    bitset->words[word_idx] &= ~bitmask;
    bitset->num_bits_marked--;
    
    // Update free_word_index if needed for faster future searches
    if (word_idx < bitset->free_word_index) {
      bitset->free_word_index = word_idx;
    }
  }
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

  WORD bitmask = (WORD)1 << bit_idx;

  bitset->words[word_idx] ^= bitmask;
  if ((bitset->words[word_idx] & bitmask) == 1) {
    bitset->num_bits_marked++;
  } else {
    bitset->num_bits_marked--;
  }
}

bool check_bit(BitSet *bitset, size_t index) {
  // if index is not in range return false
  if (index >= bitset->num_bits) {
    return false;
  }

  // figuring out what word contains the bit to flip
  size_t word_idx = calculate_word_idx(index);
  // the index within the word that needs to be changed
  size_t bit_idx = calculate_bit_idx(index);

  return (bitset->words[word_idx] >> bit_idx) & 1;
}

// Optimized find_first_unmarked_bit using CPU intrinsics
ssize_t find_first_unmarked_bit(BitSet *bitset) {
  // Start from the free_word_index for better cache locality
  size_t start_word = bitset->free_word_index;
  
  // Fast path: check if we're already at the end
  if (__builtin_expect(start_word >= bitset->num_words, 0)) {
    return -1;
  }
  
  // Check words starting from free_word_index
  for (size_t word_idx = start_word; word_idx < bitset->num_words; word_idx++) {
    WORD word = bitset->words[word_idx];
    
    // Skip full words quickly
    if (__builtin_expect(word == MAX_WORD_SIZE, 0)) {
      continue;
    }
    
    // Handle last word specially to avoid checking unused bits
    if (__builtin_expect(word_idx == bitset->num_words - 1 && bitset->last_word_bits != 0, 0)) {
      // Mask out unused bits in the last word
      WORD mask = ((WORD)1 << bitset->last_word_bits) - 1;
      word |= ~mask; // Set unused bits to 1 (marked)
      
      if (word == MAX_WORD_SIZE) {
        break; // No free bits in last word
      }
    }
    
    // Use intrinsic to find first zero bit (unmarked bit)
    WORD inverted_word = ~word;
    unsigned int bit_pos = __builtin_ctzll(inverted_word);
    
    // Calculate final bit index
    size_t bit_index = word_idx * BITS_PER_WORD + bit_pos;
    
    // Double-check we're not returning an invalid bit in the last word
    if (__builtin_expect(bit_index >= bitset->num_bits, 0)) {
      break;
    }
    
    // Update free_word_index for next search
    bitset->free_word_index = word_idx;
    
    return (ssize_t)bit_index;
  }
  
  // No unmarked bits found
  bitset->free_word_index = bitset->num_words;
  return -1;
}

bool all_bits_marked(BitSet *bitset) {
  return bitset->num_bits_marked == bitset->num_bits;
}

bool all_bits_unmarked(BitSet *bitset) { return bitset->num_bits_marked == 0; }

void clear_bitset(BitSet *bitset) {
  // getting the number of words
  size_t num_words = bitset->num_words;

  // zeroing out buffer
  memset(bitset->words, 0, num_words * sizeof(WORD));

  // setting unused bits to 1 in the last word
  if (bitset->last_word_bits != 0) {
    bitset->words[num_words - 1] = unused_bit_mask(bitset->last_word_bits);
  }
  
  // Reset counters
  bitset->num_bits_marked = 0;
  bitset->free_word_index = 0;
}

void print_bitset(BitSet *bitset) {
  // iterate through all the bits
  for (size_t i = 0; i < bitset->num_bits; i++) {
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
