#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

// The type of allocator used to make an allocation,
// this is used when freeing memory to determine
// what allocator to use to free the memory
typedef enum {
  BIN_ALLOCATION_TYPE,
  FREE_LIST_ALLOCATION_TYPE,
  HUGE_ALLOCATION_TYPE,
} AllocationType ;

// This holds metadata about the type of allocation that was made.
// It must be the first field in any metadata struct for a region of
// memory
typedef struct {
  AllocationType allocation_type;
} AllocationHeader;

// Equivalant to malloc
void *dmalloc(size_t size);

// Equivalent to calloc
void *dcalloc(size_t num, size_t size);

// Equivalent to realloc
void *drealloc(void *ptr, size_t new_size);

// Equivalent to free
void dfree(void *ptr);

size_t num_bins();

#endif
