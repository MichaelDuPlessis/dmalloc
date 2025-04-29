#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

// The type of allocator used to make an allocation,
// this is used when freeing memory to determine
// what allocator to use to free the memory
typedef enum {
  BIN_ALLOCATION_TYPE,
  FREE_LIST_ALLOCATION_TYPE,
  MMAP_ALLOCATION_TYPE,
} AllocationType ;

// This holds metadata about the type of allocation that was made.
// It must be the first field in any metadata struct for a region of
// memory
typedef struct {
  AllocationType allocation_type;
} AllocationHeader;

// Allocates memory
void *dmalloc(size_t size);

// Frees memory
void dfree(void *ptr);

#endif
