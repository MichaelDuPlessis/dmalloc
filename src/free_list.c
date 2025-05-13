#include "free_list.h"
#include "allocator.h"
#include "mmap_allocator.h"
#include <stddef.h>

// A header for a block of allocated memory
typedef struct {
  // The amount of memory allocated for a block
  // including the header
  size_t size;
} AllocHeader;

// A block of memory
typedef struct Block {
  // The amount of memory available that can be allocated
  // this includes the memory that the block occupies
  size_t size;
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
} Chunk;

// The head of the chunks
static Chunk *chunk_head = NULL;

// initializes a block
static inline void init_block(Block *block, size_t size, Block *next) {
  block->size = size;
  block->next = next;
}

static inline void init_alloc_header(AllocHeader *header, size_t size) {
  header->size = size;
}

void *free_list_alloc(size_t size) {
  Chunk *current_chunk = chunk_head;

  // Iterate over chunks
  while (current_chunk) {
    // if there is enough space left in the chunk
    Block *current_block = current_chunk->block_head;

    // iterating over blocks
    while (current_block) {
      // if there is enough space in the node allocate memory 
      if (current_block->size >= size) {
        // since there is enough memory split block and allocate memory
        AllocHeader *header = (AllocHeader *)current_block;
        void *ptr = (void *)(header + 1); // moving to just after the BlockHeader

        // the amount of memory remaining
        size_t remaining = current_block->size - (sizeof(AllocHeader) + size);

        // if there is enough space create a new block
        // TODO: If the amount of memory left is less than or equal to
        // the amount of memory the bin allocator uses this can be skipped
        if (remaining > sizeof(Block)) {
          // since there is space allocate memory
          Block *new_block = (Block *)((char *)header + sizeof(AllocHeader) + size);
          init_block(new_block, remaining, current_block->next);

          // this is for later when the header's size is assigned
          remaining = 0;
        }

        // setting headers parameters
        init_alloc_header(header, sizeof(AllocHeader) + size + remaining);

        return ptr;
      }
    }
  }
}
