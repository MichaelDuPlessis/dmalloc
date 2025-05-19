#include "allocator.h"
#include "bin.h"
#include "bitset.h"
#include "free_list.h"
#include "huge.h"
#include "mmap_allocator.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// The number of bins that we want
#define NUM_BINS 4

// the maximum sized allocation that can fit into a bin
const size_t MAX_BIN_SIZE = 1 << NUM_BINS;

// This is contains the metadata for the allocator
struct {
  // the bins for small objects
  BinManager bins[NUM_BINS];
} allocator = {.bins = {{.head = NULL, .bin_size = 1},
                        {.head = NULL, .bin_size = 2},
                        {.head = NULL, .bin_size = 4},
                        {.head = NULL, .bin_size = 8}}};

// gets the kind of allocation that was made
// must pass in memory that points to the start of a page
static inline AllocationType get_allocation_type(void *page_start) {
  AllocationHeader *header = (AllocationHeader *)page_start;
  return header->allocation_type;
}

// gets the size of an allocation
static size_t get_allocation_size(void *ptr) {
  void *page_start = calculate_page_start(ptr);
  AllocationType type = get_allocation_type(page_start);

  switch (type) {
  case BIN_ALLOCATION_TYPE:
    return bin_manager_size((BinManager *)page_start);
  case FREE_LIST_ALLOCATION_TYPE:
    return free_list_size(ptr);
  case HUGE_ALLOCATION_TYPE:
    return huge_size(page_start);
  }
}

// finding which bin an allocation belongs to
static inline size_t bin_index(size_t size) {
  // TODO: maybe don't use size_t it may not be necessary
  size_t bin = 0;
  size_t power = 1;
  while (power < size) {
    power <<= 1;
    bin++;
  }
  return bin;
}

void *dmalloc(size_t size) {
  // if size fits into a bin
  if (size <= MAX_BIN_SIZE) {
    // getting the bin index for the allocation
    size_t index = bin_index(size);

    // allocating from bin
    return bin_manager_alloc(&allocator.bins[index]);
  }

  // if size is larger than the biggest bin
  // but less than a page
  if (size < PAGE_SIZE) {
    // if not valid just allocate from the free list
    return free_list_alloc(size);
  }

  // if size is large enough just use a whole page to allocate it
  return huge_alloc(size);
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
  // determining what allocator was used to allocate the memory
  void *page_start = calculate_page_start(ptr);
  AllocationType type = get_allocation_type(page_start);

  switch (type) {
  case BIN_ALLOCATION_TYPE:
    bin_manager_free(ptr);
    break;
  case FREE_LIST_ALLOCATION_TYPE:
    free_list_free(ptr);
    break;
  case HUGE_ALLOCATION_TYPE:
    huge_free(ptr);
    break;
  }
}

// frees all memory at program exit
void free_all_memory() {
  for (size_t i = 0; i < NUM_BINS; i++) {
    bin_manager_free_all(&allocator.bins[i]);
  }
}

size_t num_bins() {
  Bin *current = allocator.bins[2].head;
  size_t amount = 0;
  while (current) {
    // printf("Bin: %zu\n", amount);
    // print_bitset(&current->bitset);
    // amount++;
    clear_bitset(&current->bitset);
    current = current->next;
  }

  // free_all_memory();

  return amount;
}
