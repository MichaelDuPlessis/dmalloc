#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

// Define compiler optimization attributes
#define DMALLOC_HOT __attribute__((hot))
#define DMALLOC_COLD __attribute__((cold))
#define DMALLOC_INLINE __attribute__((always_inline))
#define DMALLOC_NOINLINE __attribute__((noinline))
#define DMALLOC_PURE __attribute__((pure))
#define DMALLOC_CONST __attribute__((const))
#define DMALLOC_MALLOC __attribute__((malloc))

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
DMALLOC_HOT DMALLOC_MALLOC void *dmalloc(size_t size);

// Equivalent to calloc
DMALLOC_HOT DMALLOC_MALLOC void *dcalloc(size_t num, size_t size);

// Equivalent to realloc
DMALLOC_HOT DMALLOC_MALLOC void *drealloc(void *ptr, size_t new_size);

// Equivalent to free
DMALLOC_HOT void dfree(void *ptr);

DMALLOC_PURE size_t num_bins();

#endif
