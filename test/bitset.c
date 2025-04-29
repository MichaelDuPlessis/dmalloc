#include "test.h"
#include "../src/bitset.h"
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void bitset_test() {
  const size_t num_bits = 64;
  const size_t bitset_size = size_of_bitset(num_bits);

  printf("Size of BitSet: %zu\n", bitset_size);

  // Initialize a bitset with 64 bits
  BitSet *bitset = malloc(bitset_size);
  init_bitset(bitset, num_bits);

  // Test 1: Check that all bits are initially set to 0
  print_bitset(bitset); // Should print the bitset in human-readable format
  for (size_t i = 0; i < bitset->num_bits; i++) {
    assert(check_bit(bitset, i) == false);
  }

  // Test 2: Set some bits
  mark_bit(bitset, 0);
  mark_bit(bitset, 5);
  mark_bit(bitset, 63);

  // Check that the bits we set are indeed set
  assert(check_bit(bitset, 0) == true);
  assert(check_bit(bitset, 5) == true);
  assert(check_bit(bitset, 63) == true);

  // Test 3: Clear a bit
  unmark_bit(bitset, 5);

  // Check that the bit is cleared
  assert(check_bit(bitset, 5) == false);

  // Test 4: Flip a bit
  flip_bit(bitset, 0);  // Flip the 0th bit
  flip_bit(bitset, 63); // Flip the 63rd bit

  // Check that the bits are flipped correctly
  assert(check_bit(bitset, 0) == false);
  assert(check_bit(bitset, 63) == false);

  // Test 5: First unmarked bit
  ssize_t first_free = find_first_unmarked_bit(bitset);
  assert(first_free == 0);

  // Test 6: Mark a bit (using mark_bit)
  mark_bit(bitset, 10);
  assert(check_bit(bitset, 10) == true);

  // Test 7: Set the last bit and check that it does not exceed bounds
  mark_bit(bitset, 63);
  assert(check_bit(bitset, 63) == true);

  // Test 8: Try to set an invalid bit (out of range)
  mark_bit(bitset, 64); // Should not do anything, out of range

  // Test 9: Print the bitset
  print_bitset(bitset); // Should print the bitset in human-readable format

  free(bitset);
}
