#include "distributer.h"
#include "block.h"
#include "mmap_allocator.h"
#include <stddef.h>
#include <stdlib.h>

// specifies how many pages to start with
const size_t INITIAL_NUM_PAGES = 1;

struct {
  Chunk chunk_head;
} distributer;

// initializes the distributer
void init_distributer() {
  // registering deinit function to run at program exit to clean up any memory
  atexit(deinit_distributer);

  // getting memory
  MmapAllocation allocation = mmap_alloc(INITIAL_NUM_PAGES);
  void* ptr = allocation.ptr;
  size_t size = allocation.size;

  // creating the free list for the initial allocation
  *(Chunk *)ptr = (Chunk) {
    .next = NULL,
    .ptr = (BlockNode *)((Chunk *)ptr + 1),
    size = size - sizeof(Chunk),
    .allocation = allocation,
  };
}

// deallocates all memory attached to the distributer
void deinit_distributer() {
  // saving the head for later so values can be set to NULL
  Chunk head = distributer.chunk_head;
  Chunk *current = &distributer.chunk_head;

  while (current) {
    mmap_free(current->allocation);

    current = current->next;
  }

  head.next = NULL;
  head.allocation.ptr = NULL;
  head.allocation.size = 0;
  head.size = 0;
  head.ptr = NULL;
}
