#include "allocator.h"
#include "bin.h"
#include "free_list.h"
#include "huge.h"
#include "mmap_allocator.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define ONLY_SMALL

// gets the kind of allocation that was made
// must pass in memory that points to the start of a page
static inline AllocationType get_allocation_type(AllocationHeader *header) {
  return header->allocation_type;
}

// gets the size of an allocation
static inline size_t get_allocation_size(void *ptr) {
  void *page_start = calculate_page_start(ptr);
  AllocationType type = get_allocation_type(page_start);

  switch (type) {
  case BIN_ALLOCATION_TYPE:
    return bin_size(page_start);
  case FREE_LIST_ALLOCATION_TYPE:
    return free_list_size(ptr);
  case HUGE_ALLOCATION_TYPE:
    return huge_size(page_start);
  }
}

void *dmalloc(size_t size) {
  // if size fits into a bin
  if (size <= MAX_BIN_SIZE) {
    // allocating from bin
    void *ptr = bin_alloc(size);
    return ptr;
  }

#ifndef ONLY_SMALL
  // if size is larger than the biggest bin
  // but less than a page
  if (size < (PAGE_SIZE / 2)) {
    // if not valid just allocate from the free list
    return free_list_alloc(size);
  }

  // if size is large enough just use a whole page to allocate it
  return huge_alloc(size);
#endif

#ifdef ONLY_SMALL
  return malloc(size);
#endif
}

void *dcalloc(size_t num, size_t size) {
  size_t amount = num * size;

  void *ptr = dmalloc(amount);
  memset(ptr, 0, amount);

  return ptr;
}

// The allocator does not curretly support resizing memory allocations in place
void *drealloc(void *ptr, size_t new_size) {
  if (ptr == NULL) {
    return dmalloc(new_size);
  }

  if (new_size == 0) {
    dfree(ptr);
    return NULL;
  }

  // getting size of what was allocated before
  size_t size = get_allocation_size(ptr);

  // if the sizes are the same do nothing
  if (size == new_size) {
    return ptr;
  }

  // otherwise allocate new memory of the requested size
  void *new_ptr = dmalloc(new_size);
  // getting the smaller of the two sizes to copy bytes over
  size_t amount_to_copy = size < new_size ? size : new_size;

  memcpy(new_ptr, ptr, amount_to_copy);

  // after data has been copied free the old memory
  dfree(ptr);

  return new_ptr;
}

void dfree(void *ptr) {
#ifndef ONLY_SMALL
  // determining what allocator was used to allocate the memory
  void *page_start = calculate_page_start(ptr);
  AllocationType type = get_allocation_type(page_start);

  switch (type) {
  case BIN_ALLOCATION_TYPE:
    bin_free(ptr, page_start);
    break;
  case FREE_LIST_ALLOCATION_TYPE:
    free_list_free(ptr, page_start);
    break;
  case HUGE_ALLOCATION_TYPE:
    huge_free(ptr);
    break;
  }
#endif

#ifdef ONLY_SMALL
  struct Bin *bin = allocated_by_bin(ptr);
  if (bin != NULL) {
    bin_free(ptr, bin);
  } else {
    free(ptr);
  }
#endif
}
