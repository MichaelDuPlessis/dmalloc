#ifndef BIN_H
#define BIN_H

#include <stddef.h>
#include "allocator.h"
#include "bitset.h"

// The number of bins that we want
#define NUM_BINS 4

// the maximum sized allocation that can fit into a bin
#define MAX_BIN_SIZE (1 << (NUM_BINS - 1))

// Forward declaration since the implementor does not need to know the inner workings
struct Bin;

// Allocators memory to a bin and returns a pointer to the bin
void *bin_alloc(size_t size);

// Frees memory from the bin containing the pointer
void bin_free(void *ptr, struct Bin *bin);

// The size of blocks of memory that the bin allocates
size_t bin_size(struct Bin *bin);

#endif
