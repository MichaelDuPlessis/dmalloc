#ifndef BIN_H
#define BIN_H

#include <stddef.h>
#include "allocator.h"
#include "bitset.h"

// The number of bins that we want
#ifndef NUM_BINS
#define NUM_BINS 8
#endif

// the maximum sized allocation that can fit into a bin
#define MAX_BIN_SIZE (1 << (NUM_BINS - 1))

// Forward declaration since the implementor does not need to know the inner workings
struct Bin;

// Allocators memory to a bin and returns a pointer to the bin
DMALLOC_HOT DMALLOC_MALLOC void *bin_alloc(size_t size);

// Frees memory from the bin containing the pointer
DMALLOC_HOT void bin_free(void *ptr, struct Bin *bin);

// The size of blocks of memory that the bin allocates
DMALLOC_PURE size_t bin_size(struct Bin *bin);

// Whether this memory pointed to by the provided ptr was allocated using
// the bin allocator. If true return a pointer to the Bin in which it belongs if
// false return null
DMALLOC_PURE struct Bin *allocated_by_bin(void *ptr);

#endif

