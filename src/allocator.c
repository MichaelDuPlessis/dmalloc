#include "allocator.h"
#include "bin.h"
#include "distributer.h"
#include <stddef.h>
#include <stdio.h>

// The number of bins that we want
const size_t NUM_BINS = 4;

// the maximum sized allocation that can fit into a bin
const size_t MAX_BIN_SIZE = 1 << NUM_BINS;

// This is contains the metadata for the allocator
struct {
  // the bins for small objects
  BinManager bins[NUM_BINS];
} allocator = {
    .bins = {{.head = NULL, .bin_size = 1}, {.head = NULL, .bin_size = 2}, {.head = NULL, .bin_size = 4}, {.head = NULL, .bin_size = 8}}};

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
  // seeing if there is a valid bin for the allocation
  if (size > MAX_BIN_SIZE) {
    // if not valid just allocate from the free list
    return request_block(size);
  }

  // getting the bin index for the allocation
  size_t index = bin_index(size);
  // allocating from bin
  return manager_alloc(&allocator.bins[index]);
}

void dfree(void *ptr) {
  // determining if the memory was allocated from a free list or from a bin
  // return_block(ptr);

  manager_free(ptr);
}

// frees all memory at program exit
void free_all_memory() {
  for (size_t i = 0; i < NUM_BINS; i++) {
    manager_free_all(&allocator.bins[i]);
  } 
}
