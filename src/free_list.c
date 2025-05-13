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
  struct Chunk *next;
  // Where the first free block of memory is
  Block *block_head;
} Chunk;

// The head of the chunks
static Chunk *chunk_head = NULL;

// initializes a Block
static inline void init_block(Block *block, size_t size, Block *next) {
  block->size = size;
  block->next = next;
}

// initializes and AllocHeader
static inline void init_alloc_header(AllocHeader *header, size_t size) {
  header->size = size;
}

// Allocates memory using mmap and creates a new chunk and initializes it
static inline Chunk *new_chunk() {
  MmapAllocation allocation = mmap_alloc(1);
  // this is the start of the chunk
  Chunk *chunk = (Chunk *)allocation.ptr;

  // creating block head
  Block *block = (Block *)(chunk + 1);
  init_block(block, allocation.size - sizeof(Block), NULL);

  *chunk = (Chunk){
      .header = FREE_LIST_ALLOCATION_TYPE,
      .mmap_allocation = allocation,
      .next = NULL,
      .block_head = block,
  };

  return chunk;
}

void *free_list_alloc(size_t size) {
  Chunk *current_chunk = chunk_head;

  while (current_chunk) {
    Block *prev_block = NULL;
    Block *current_block = current_chunk->block_head;

    while (current_block) {
      if (current_block->size >= size + sizeof(AllocHeader)) {
        size_t total_size = sizeof(AllocHeader) + size;
        size_t remaining = current_block->size - total_size;

        void *ptr;
        if (remaining > sizeof(Block)) {
          // Split the block
          Block *new_block = (Block *)((char *)current_block + total_size);
          init_block(new_block, remaining, current_block->next);

          if (prev_block) {
            prev_block->next = new_block;
          } else {
            current_chunk->block_head = new_block;
          }

          current_block->size = total_size; // Resize current block
        } else {
          // Use the whole block
          total_size = current_block->size;

          if (prev_block) {
            prev_block->next = current_block->next;
          } else {
            current_chunk->block_head = current_block->next;
          }
        }

        AllocHeader *header = (AllocHeader *)current_block;
        init_alloc_header(header, total_size);
        ptr = (void *)(header + 1);
        return ptr;
      }

      prev_block = current_block;
      current_block = current_block->next;
    }

    current_chunk = current_chunk->next;
  }

  // No suitable block found — create new chunk
  Chunk *chunk = new_chunk();
  chunk->next = chunk_head;
  chunk_head = chunk;

  return free_list_alloc(size); // try again
}
