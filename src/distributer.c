#include "distributer.h"
#include "block.h"
#include "mmap_allocator.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// specifies how many pages to start with
const size_t INITIAL_NUM_PAGES = 1;

// checking if the distributer has been initialized
static bool distributer_initalized = false;

struct {
  Chunk *chunk_head;
} distributer;

// initializes the distributer
void init_distributer() {
  // registering deinit function to run at program exit to clean up any memory
  atexit(deinit_distributer);

  // getting memory
  MmapAllocation allocation = mmap_alloc(INITIAL_NUM_PAGES);
  void *ptr = allocation.ptr;
  size_t size = allocation.size;

  // creating block head
  BlockNode *block_head = (BlockNode *)((Chunk *)ptr + 1);
  *block_head = (BlockNode){
      .next = NULL,
      size = size - sizeof(Chunk),
  };

  // creating chunk head
  Chunk* chunk_head = (Chunk *)ptr;
  *chunk_head = (Chunk){
      .next = NULL,
      .block_head = block_head,
      size = size - sizeof(Chunk),
      .allocation = allocation,
  };

  distributer.chunk_head = chunk_head;
}

// deallocates all memory attached to the distributer
void deinit_distributer() {
  // saving the head for later so values can be set to NULL
  Chunk *current = distributer.chunk_head;

  while (current) {
    Chunk* next = current->next;
    mmap_free(current->allocation);

    current = next;
  }

  distributer.chunk_head = NULL;
}

void *request_block(size_t size) {
  // initializing the distributer if it has not been already
  if (!distributer_initalized) {
    init_distributer();
    distributer_initalized = true;
  }

  Chunk *curr_chunk = distributer.chunk_head;

  // iterating over chunks
  while (curr_chunk) {
    // iterating over block nodes
    BlockNode *curr_block_node = curr_chunk->block_head;

    // iterating over block nodes
    while (curr_block_node) {
      // seeing if there is enough space in the block to allocate the memory
      if (curr_block_node->size >= size + sizeof(BlockHeader)) {
        // setting the next node
        // getting the next pointer
        BlockNode *next = curr_block_node->next;
        // the amount of memory the header and the allocation takes
        size_t total_allocation_size = sizeof(BlockHeader) + size;
        BlockNode new_head = {
            // convert to char since it is gaurenteed to be 1 byte
            .next = next,
            .size = curr_block_node->size - total_allocation_size,
        };

        // saving the new head to memory
        curr_chunk->block_head =
            (BlockNode *)((char *)curr_block_node + total_allocation_size);

        // creating header for the memory to be allocated
        BlockHeader header = {
            .size = size,
        };

        // assigning the block header
        BlockHeader *ptr = (BlockHeader *)curr_block_node;
        *ptr = header;
        // returning a pointer to the data
        return (void *)(ptr + 1);
      }
    }
  }

  return NULL;
}
