#ifndef DISTRIBUTER_H
#define DISTRIBUTER_H

#include "block.h"
#include "mmap_allocator.h"
#include <stddef.h>

// the header for a chunk of memory that the distributers stores
typedef struct Chunk {
  // the next chunk of memory
  struct Chunk* next;
  // where the chunk starts
  BlockNode* ptr;
  // the amount of memory the chunk has left after the header
  size_t size;
  // the original allocation
  MmapAllocation allocation;
} Chunk;

// initializes the distributer
void init_distributer();

// deinitializes the distributer
void deinit_distributer();

#endif
