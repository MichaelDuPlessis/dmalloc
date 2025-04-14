#include "allocator.h"
#include "bin.h"
#include "distributer.h"
#include <stddef.h>
#include <stdio.h>

// The number of bins that we want
const size_t BINS = 4;

// the maximum sized allocation that can fit into a bin
const size_t MAX_BIN_SIZE = 1 << BINS;

// This is contains the metadata for the allocator
struct {
  // the bins for small objects
  BinManager bins[BINS];
} allocator = {
    .bins = {{.head = NULL}, {.head = NULL}, {.head = NULL}, {.head = NULL}}};

// finding which bin an allocation belongs to
size_t bin_index(size_t size) {
  // TODO: Maybe don't use size_t it isn't necessary
  size_t bin = 0;
  size--;
  while (size >>= 1)
    bin++;
  return bin;
}

void *malloc(size_t size) {
  // seeing if there is a valid bin for the allocation
  if (size > MAX_BIN_SIZE) {
    // if not valid just allocate from the free list
    return request_block(size);
  }

  // getting the bin index for the allocation
  size_t index = bin_index(size);
  // allocating from bin
  return bin_alloc(&allocator.bins[index], 1 << index);
}

void free(void *ptr) { return_block(ptr); }
