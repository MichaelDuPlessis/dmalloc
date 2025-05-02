#include "distributer.h"
#include "block.h"
#include "mmap_allocator.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// This type is used to represent a byte
#define BYTE char

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
  size_t available_memory = size - sizeof(Chunk);
  Chunk *chunk_head = (Chunk *)ptr;
  *chunk_head = (Chunk){
      .next = NULL,
      .block_head = block_head,
      size = available_memory,
      .mmap_allocation = allocation,
      .free_bytes = available_memory,
  };

  distributer.chunk_head = chunk_head;
}

// deallocates all memory attached to the distributer
void deinit_distributer() {
  // saving the head for later so values can be set to NULL
  Chunk *current = distributer.chunk_head;

  while (current) {
    Chunk *next = current->next;
    mmap_free(current->mmap_allocation);

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
  // the amount of memory the header and the allocation takes
  size_t total_allocation_size = sizeof(BlockHeader) + size;

  // iterating over chunks
  while (curr_chunk && curr_chunk->free_bytes >= total_allocation_size) {
    // iterating over block nodes
    BlockNode *curr_block_node = curr_chunk->block_head;

    // iterating over block nodes
    while (curr_block_node) {
      // seeing if there is enough space in the block to allocate the memory
      if (curr_block_node->size >= size + sizeof(BlockHeader)) {
        // setting the next node
        // getting the next pointer
        BlockNode *next = curr_block_node->next;

        // the amount of memory left in the node after the allocation
        size_t memory_left = curr_block_node->size - total_allocation_size;

        // checking if there is enough space to create a new block node
        if (memory_left >= sizeof(BlockNode)) {
          // convert to char since it is gaurenteed to be 1 byte
          BlockNode *new_head =
              (BlockNode *)((BYTE *)curr_block_node + total_allocation_size);
          *new_head = (BlockNode){
              .next = next,
              .size = curr_block_node->size - total_allocation_size,
          };

          // assigning new head
          curr_chunk->block_head = new_head;
        } else {
          // if there is not enough memory to create a new block use all the
          // memory for the allocation
          curr_chunk->block_head = next;
          size = curr_block_node->size;
        }

        // creating header for the memory to be allocated
        BlockHeader *header = (BlockHeader *)curr_block_node;
        *header = (BlockHeader){
            .size = size,
        };

        // keeping track of how much has been allocated
        curr_chunk->free_bytes += total_allocation_size;
        // returning a pointer to the data which will just be after the header
        return (void *)(header + 1);
      }

      // if this block does not have enough space go to the next one
      curr_block_node = curr_block_node->next;
    }

    // if this chunk cannot satisfy the request go to the next one
    curr_chunk = curr_chunk->next;
  }

  // TODO: Add logic to allocate more memory from mmap

  // this means we are out of memory
  return NULL;
}

// checks if a pointer belongs to a chunk
static bool pointer_belongs_to_chunk(void *ptr, Chunk *chunk) {
  return (ptr >= chunk->mmap_allocation.ptr) &&
         (ptr < (void *)((BYTE *)chunk->mmap_allocation.ptr +
                         chunk->mmap_allocation.size));
}

// frees the memory from a chunk
static void free_memory_from_chunk(void *ptr, Chunk *chunk) {
  // get the amount of memory used by the header and the allocation
  BlockHeader *header = (BlockHeader *)ptr - 1;
  size_t total_memory_used = header->size + sizeof(BlockHeader);

  // replacing the header with a node
  BlockNode *node = (BlockNode *)header;
  *node = (BlockNode){
    .next = chunk->block_head,
    .size = total_memory_used,
  };

  // making the node the new head of the free list
  chunk->block_head = node;
  chunk->free_bytes += node->size;
}

void return_block(void *ptr) {
  // first check if the memory was allocated by this allocator
  Chunk *curr_chunk = distributer.chunk_head;

  while (curr_chunk) {
    if (pointer_belongs_to_chunk(ptr, curr_chunk)) {
      free_memory_from_chunk(ptr, curr_chunk);
      return;
    }
  }

  // if memory was not allocated by this allocator exit the program 
  exit(EXIT_FAILURE);
}
