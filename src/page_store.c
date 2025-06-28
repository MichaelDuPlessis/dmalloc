#include "page_store.h"
#include "mmap_allocator.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// The number of papes to keep cached
#ifndef STORE_SIZE
#define STORE_SIZE 4
#endif

// The store that holds the pages

static MmapAllocation store[STORE_SIZE] = {0};

// Determines if the MmapAllocation is 0 initialized 
static inline bool is_zero_initalized(MmapAllocation allocation) {
  return allocation.ptr == NULL && allocation.size == 0;
}

MmapAllocation retrieve_page() {
  // find first free page
  for (size_t i = 0; i < STORE_SIZE; i++) {
    MmapAllocation allocation = store[i];

    // if a page is found
    if (!is_zero_initalized(allocation)) {
      // Mark spot as not available
      store[i] = (MmapAllocation){0};

      // return allocation
      return allocation;
    }
  }

  // if no free spot is found then new pages need to be allocated
  // allocate pages plus an extra one for memory that has to be allocated now
  size_t pages_to_allocated = STORE_SIZE + 1;
  MmapAllocation allocations = mmap_alloc(pages_to_allocated);
  // the pointer to the beginning of the memory region
  char *ptr = allocations.ptr;

  // getting the size of a page
  size_t page_size = get_page_size();

  // all pages are allocated at once for effiency so the headers now need to be added
  for (size_t i = 1; i < pages_to_allocated; i++) {    
    store[i] = (MmapAllocation) {
      .ptr = (void *)(ptr + page_size * i),
      .size = page_size,
    };
  }

  MmapAllocation allocation = {
   .ptr = ptr,
   .size = page_size, 
  };

  return allocation; 
}

void store_page(MmapAllocation allocation) {
  // finding free slot to store page
  for (size_t i = 0; i < STORE_SIZE; i++) {
    MmapAllocation allocation = store[i];

    // if a page is found
    if (is_zero_initalized(allocation)) {
      // Mark spot as not available
      store[i] = allocation;
      return;
    }
  }

  // if no free spot is found deallocate memory
  mmap_free(allocation);
}
