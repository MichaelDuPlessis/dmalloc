#include "bin.h"
#include "bitset.h"
#include "mmap_allocator.h"

// Calculates the number of blocks that can fit in some amount of memory
size_t calculate_num_blocks(size_t block_size, size_t total_memory) {
  return total_memory / block_size;
}

// Calculates the number of bits needed for the bitset
size_t calculate_bitset_size(size_t block_size) {
  // the amount of memory availble
  // since I don't want to include th bitsets values
  size_t total_memory_available = PAGE_SIZE - (sizeof(Bin) - sizeof(BitSet));

  size_t total_blocks =
      calculate_num_blocks(block_size, total_memory_available);
  // size of bitset required
  size_t bitset_size = size_of_bitset(total_blocks);

  // after accounting for the size of the bitset figure out how many blocks
  // can now be allocated
  size_t new_total_blocks =
      calculate_num_blocks(block_size, total_memory_available - bitset_size);

  // if the new total number of blocks is the same as the old we have converged
  while (new_total_blocks != total_blocks) {
    new_total_blocks =
        calculate_num_blocks(block_size, total_memory_available - bitset_size);

    // getting the new
    bitset_size = size_of_bitset(new_total_blocks);
    total_blocks = new_total_blocks;
  }

  return total_blocks;
}

void init_bin(Bin *bin, size_t bin_size) {
  // setting the size of the bin
  bin->bin_size = bin_size;
  bin->next = NULL;

  // calculating the size of the bitset
  size_t num_bits = calculate_bitset_size(bin_size);

  // initializing bitset
  // since the bitset has to at the end this is a clever way to get its start
  init_bitset(((BitSet *)(bin + 1)) - sizeof(BitSet), num_bits);

  // Setting where the initial memory used for allocation begins
  bin->ptr = (void *)(sizeof(Bin) + size_of_bitset(num_bits) - sizeof(BitSet));
}
