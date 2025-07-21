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
  // TODO: I think there is a better way to do this
  bitset->last_word_bits = num_bits % BITS_PER_WORD;

  clear_bitset(bitset);
}

void mark_bit(BitSet *bitset, size_t index) {
  // since this is almost never true we can
  // give the compiler a hint
  if (__builtin_expect(index >= bitset->num_bits, 0))
    return;

  // figuring out what word contains the bit to flip
  size_t word_idx = calculate_word_idx(index);
  // the index within the word that needs to be changed
  size_t bit_idx = calculate_bit_idx(index);

  // since the index must be in a valid range and the unused bits are always
  // on marking does nothing and the code below to check is not needed

  WORD bitmask = (WORD)1 << bit_idx;
  // checking if bit is already marked
  if ((bitset->words[word_idx] & bitmask) == 0) {
    // if so a new bit has been marked
    bitset->num_bits_marked++;
  }

  bitset->words[word_idx] |= bitmask;

  // if the current index and the last free index are the same
  // and the current word is full than move to the next one
  if (word_idx == bitset->free_word_index &&
      bitset->words[word_idx] == MAX_WORD_SIZE) {
    bitset->free_word_index++;
  }
}

void unmark_bit(BitSet *bitset, size_t index) {
  if (index >= bitset->num_bits)
    return;

  // figuring out what word contains the bit to flip
  size_t word_idx = calculate_word_idx(index);
  // the index within the word that needs to be changed
  size_t bit_idx = calculate_bit_idx(index);

  // checking if index is in the last word and if there are any unused bits
  // making sure that we are not trying to unset one of the last invalid bits
  if (word_idx == (bitset->num_bits / BITS_PER_WORD) &&
      bitset->last_word_bits != 0 && bit_idx >= bitset->last_word_bits) {
    return;
  }

  WORD bitmask = (WORD)1 << bit_idx;
  // checking if bit is already marked
  if ((bitset->words[word_idx] & bitmask) == 1) {
    // if the bit was marked less bits than have been marked
    bitset->num_bits_marked--;
  }
  bitset->words[word_idx] &= ~bitmask;

  // setting the index for the word with free bits
  // only update it if the index is less than the current one
  // that way all bits to the left are known to be marked
  if (word_idx < bitset->free_word_index) {
    bitset->free_word_index = word_idx;
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

// ssize_t is used because it can reprsent -1 to show no bit found
ssize_t find_first_unmarked_bit(BitSet *bitset) {
  // Early return if all bits are marked
  if (bitset->num_bits_marked == bitset->num_bits) {
    return -1;
  }

  size_t num_words = bitset->num_words;
  size_t word_idx = bitset->free_word_index;

  // Process words
  while (word_idx < num_words) {
    WORD word = bitset->words[word_idx];

    // If not all bits are marked in this word
    if (word != MAX_WORD_SIZE) {
      // Find position of first unmarked bit using compiler intrinsic
      unsigned int bit_pos;

#if defined(__GNUC__) || defined(__clang__)
      // GCC and Clang provide __builtin_ctzl
      bit_pos = __builtin_ctzl(~word);
#elif defined(_MSC_VER)
      // MSVC has _BitScanForward
      unsigned long index;
      _BitScanForward(&index, ~word);
      bit_pos = index;
#else
      // Fallback implementation for other compilers
      WORD temp = ~word;
      bit_pos = 0;
      while ((temp & 1) == 0) {
        temp >>= 1;
        bit_pos++;
      }
#endif

      // Calculate absolute bit position
      size_t index = word_idx * BITS_PER_WORD + bit_pos;

      // Ensure the index is within valid range
      if (index < bitset->num_bits) {
        return (ssize_t)index;
      }
    }

    word_idx++;

// Optional: prefetch next word if not at the end
#if defined(__GNUC__) || defined(__clang__)
    if (word_idx < num_words - 1) {
      __builtin_prefetch(&bitset->words[word_idx + 1], 0, 0);
    }
#endif
  }

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
  // TODO: Find a way to get rid of this if statement
  if (bitset->last_word_bits != 0) {
    bitset->words[num_words - 1] = unused_bit_mask(bitset->last_word_bits);
  }
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
