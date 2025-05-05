#include "allocator.h"
#include "bin.h"
#include "bitset.h"
#include "distributer.h"
#include "huge.h"
#include "mmap_allocator.h"
#include <stddef.h>
#include <stdio.h>

// The number of bins that we want
#define NUM_BINS 4

// the maximum sized allocation that can fit into a bin
const size_t MAX_BIN_SIZE = 1 << NUM_BINS;

// This is contains the metadata for the allocator
struct {
  // the bins for small objects
  BinManager bins[NUM_BINS];
} allocator = {.bins = {{.head = NULL, .bin_size = 1},
                        {.head = NULL, .bin_size = 2},
                        {.head = NULL, .bin_size = 4},
                        {.head = NULL, .bin_size = 8}}};

// finding which bin an allocation belongs to
size_t bin_index(size_t size) {
  // TODO: maybe don't use size_t it may not be necessary
  size_t bin = 0;
  size_t power = 1;
  while (power < size) {
    power <<= 1;
    bin++;
  }
  return bin;
}

void *dmalloc(size_t size) {
  // if size fits into a bin
  if (size <= MAX_BIN_SIZE) {
    // getting the bin index for the allocation
    size_t index = bin_index(size);

    // allocating from bin
    return manager_alloc(&allocator.bins[index]);
  }

  // if size is larger than a biggest bin
  // but less than a page
  if (size < PAGE_SIZE) {
    // if not valid just allocate from the free list
    return request_block(size);
  }

  // if size is large enough just use a whole page to allocate it
  return huge_alloc(size);
}

void dfree(void *ptr) {
  // determining if the memory was allocated from a free list or from a bin
  // return_block(ptr);

  // determining what allocator was used to allocate the memory
  void *page_start = calculate_page_start(ptr);
  AllocationHeader *header = (AllocationHeader *)page_start;

  switch (header->allocation_type) {
  case BIN_ALLOCATION_TYPE:
    manager_free(ptr);
    break;
  case FREE_LIST_ALLOCATION_TYPE:
    return_block(ptr);
    break;
  case HUGE_ALLOCATION_TYPE:
    huge_free(ptr);
    break;
  }
}

// frees all memory at program exit
void free_all_memory() {
  for (size_t i = 0; i < NUM_BINS; i++) {
    manager_free_all(&allocator.bins[i]);
  }
}

size_t num_bins() {
  Bin *current = allocator.bins[2].head;
  size_t amount = 0;
  while (current) {
    // printf("Bin: %zu\n", amount);
    // print_bitset(&current->bitset);
    // amount++;
    clear_bitset(&current->bitset);
    current = current->next;
  }

  // free_all_memory();

  return amount;
}
