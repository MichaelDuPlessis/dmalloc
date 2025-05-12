#include "free_list.h"
#include "allocator.h"
#include "mmap_allocator.h"
#include <stddef.h>

// A header for a block of memory
typedef struct {
  // The amount of memory allocated for a block
  // not including the header
  size_t size;
} BlockHeader;

// A block of memory
typedef struct Block {
  // Metadata about the allocation
  BlockHeader header;
  // Where the next free block is located
  struct Block *next;
} Block;

// A chunk of memory that can be further subdivided into blocks
typedef struct Chunk {
  // Meta data about the kind of allocation
  AllocationHeader header;
  // The original allocation
  MmapAllocation mmap_allocation;
  // The next chunk of memory
  struct Chunk* next;
  // Where the first free block of memory is
  Block* block_head;
  // The amount of memory the chunk has left after the header
  size_t size;
  // The amount of memory that is free
  size_t free_bytes;
} Chunk;
