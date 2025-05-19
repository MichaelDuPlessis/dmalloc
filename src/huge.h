// This is for an allocator that allocators large amounts of memory. Large amounts of memory
// is defined as greater than a page size
#ifndef HUGE_H
#define HUGE_H

#include <stddef.h>

// Allocates a large amount of memory of at least 1 page size
void *huge_alloc(size_t size);

// Deallocates a large amount of memory allocated by hugealloc
void huge_free(void *ptr);

// Retrieves the amount of memory allocated for this allocation
// this is the amount of memory needed, not used.
// It requires a page aligned pointer
size_t huge_size(void *ptr);

#endif
