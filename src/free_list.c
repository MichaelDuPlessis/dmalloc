#include "free_list.h"
#include "allocator.h"
#include "mmap_allocator.h"
#include "page_store.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// used to calculate the alignment of some variable
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

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
  // The previous chunk of memory
  struct Chunk *prev;
  // The next chunk of memory
  struct Chunk *next;
  // Where the first free block of memory is
  Block *block_head;
} Chunk;

// The head of the chunks
static Chunk *chunk_head = NULL;

// Returns an aligned memory address
static inline void *alignment_forward(void *ptr, size_t alignment) {
  uintptr_t p = (uintptr_t)ptr;
  size_t misalignment = p & (alignment - 1);
  size_t padding = misalignment == 0 ? 0 : (alignment - misalignment);
  return (void *)(p + padding);
}

// ***CHANGED***: Renamed for clarity and made robust with a NULL check.
// Checks if the chunk has been fully coalesced into a single free block.
static inline bool is_chunk_fully_coalesced(Chunk *chunk) {
  // Calculate the total space that should be available for blocks.
  void *first_block_ptr = alignment_forward((void *)(chunk + 1), ALIGNMENT);
  size_t total_block_space = (uintptr_t)chunk + chunk->mmap_allocation.size -
                             (uintptr_t)first_block_ptr;

  // Check if there is exactly one block and it occupies all available space.
  return chunk->block_head != NULL && chunk->block_head->next == NULL &&
         chunk->block_head->size == total_block_space;
}

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
  MmapAllocation allocation = retrieve_page();
  // this is the start of the chunk
  Chunk *chunk = (Chunk *)allocation.ptr;

  // creating block head
  uintptr_t chunk_end = (uintptr_t)((char *)allocation.ptr + allocation.size);
  Block *block = (Block *)alignment_forward((void *)(chunk + 1), ALIGNMENT);
  size_t block_size = chunk_end - (uintptr_t)block;
  init_block(block, block_size, NULL);

  *chunk = (Chunk){
      .header = FREE_LIST_ALLOCATION_TYPE,
      .mmap_allocation = allocation,
      .prev = NULL,
      .next = NULL,
      .block_head = block,
  };

  return chunk;
}

// ***CHANGED***: Refactored to remove code duplication and fix logic bugs.
void *free_list_alloc(size_t size) {
  // calculating the actual amount of memory that is needed
  size = ALIGN(size);

// Label to restart the search after creating a new chunk.
start_search:;
  Chunk *current_chunk = chunk_head;

  while (current_chunk) {
    Block *prev_block = NULL;
    Block *current_block = current_chunk->block_head;

    while (current_block) {
      size_t total_size = sizeof(AllocHeader) + size;
      if (current_block->size >= total_size) {
        size_t remaining = current_block->size - total_size;

        // ***FIXED***: The condition for splitting is now correct.
        // A remaining fragment only needs to be large enough for a Block
        // struct.
        if (remaining >= sizeof(Block)) {
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
        return (void *)(header + 1);
      }

      prev_block = current_block;
      current_block = current_block->next;
    }

    current_chunk = current_chunk->next;
  }

  // No suitable block found — create new chunk
  Chunk *chunk = new_chunk();
  chunk->next = chunk_head;
  if (chunk_head != NULL) {
    chunk_head->prev = chunk;
  }
  chunk_head = chunk;

  // ***REMOVED***: The duplicated allocation logic has been removed.
  // Instead, we loop again. The new chunk is at the head and will be found
  // instantly.
  goto start_search;
}

void free_list_free(void *ptr, Chunk *chunk) {
  // first extract the header
  AllocHeader *header = (AllocHeader *)ptr - 1;

  // we then iterate over the linked list to find first free block of memory
  // just after the block that is to be deallocated
  Block *current = chunk->block_head;
  Block *previous = NULL;

  // keep iterating until a block after the header is found
  while (current && (uintptr_t)current < (uintptr_t)header) {
    previous = current;
    current = current->next;
  }

  Block *new_block = (Block *)header;

  // if current is NULL than header is at the end of the allocation
  if (current == NULL) {
    init_block(new_block, header->size, NULL);
    if (previous) {
      previous->next = new_block;
    } else {
      chunk->block_head = new_block;
    }
  }
  // if current is right after the current header than the blocks can be
  // coalesced
  else if ((char *)header + header->size == (char *)current) {
    init_block(new_block, header->size + current->size, current->next);
  } else {
    init_block(new_block, header->size, current);
  }

  // if previous is NULL it means the block of memory to be deallocated is in
  // the begining otherwise have previous point to the new block
  if (previous) {
    // if the previous block ends by the header coalesce it too
    if ((char *)previous + previous->size == (char *)new_block) {
      previous->size += new_block->size;
      previous->next = new_block->next;
    } else {
      previous->next = new_block;
    }
  } else {
    chunk->block_head = new_block;
  }

  // if there is no allocated memory left return allocation to the store
  // ***CHANGED***: Calls the safer, renamed function.
  if (is_chunk_fully_coalesced(chunk)) {
    // if the prev is null it is the head
    if (chunk->prev == NULL) {
      chunk_head = chunk->next;
      if (chunk_head != NULL) {
        chunk_head->prev = NULL;
      }
    } else {
      chunk->prev->next = chunk->next;
      // if there is a next
      if (chunk->next != NULL) {
        chunk->next->prev = chunk->prev;
      }
    }
    // return the page to the store
    store_page(chunk->mmap_allocation);
  }
}

size_t free_list_size(void *ptr) {
  AllocHeader *header = (AllocHeader *)ptr - 1;
  return header->size - sizeof(AllocHeader);
}
