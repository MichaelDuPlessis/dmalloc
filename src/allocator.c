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
  // Fast path: handle zero-size allocation
  if (__builtin_expect(size == 0, 0)) {
    // Return a small allocation for zero-size requests
    return bin_alloc(1);
  }
  
  // Fast path: if size fits into a bin (most common case)
  if (__builtin_expect(size <= MAX_BIN_SIZE, 1)) {
    return bin_alloc(size);
  }

#ifndef ONLY_SMALL
  // Medium allocations: larger than bin but smaller than half a page
  if (size < (PAGE_SIZE / 2)) {
    return free_list_alloc(size);
  }

  // Large allocations: use whole pages
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

// The allocator does not currently support resizing memory allocations in place
void *drealloc(void *ptr, size_t new_size) {
  // Fast path: handle NULL pointer (equivalent to malloc)
  if (__builtin_expect(ptr == NULL, 0)) {
    return dmalloc(new_size);
  }

  // Fast path: handle zero size (equivalent to free)
  if (__builtin_expect(new_size == 0, 0)) {
    dfree(ptr);
    return NULL;
  }

  // Get size of current allocation
  size_t current_size = get_allocation_size(ptr);
  
  // Fast path: if sizes are the same, no need to reallocate
  if (current_size == new_size) {
    return ptr;
  }
  
  // Optimization: if new size is smaller and significantly so,
  // still reallocate to avoid wasting memory
  if (new_size < current_size && new_size <= (current_size / 2)) {
    // Allocate new smaller block
    void *new_ptr = dmalloc(new_size);
    if (__builtin_expect(new_ptr == NULL, 0)) {
      // If allocation fails, return original pointer
      return ptr;
    }
    
    // Copy data to new location
    memcpy(new_ptr, ptr, new_size);
    
    // Free old memory
    dfree(ptr);
    
    return new_ptr;
  }
  
  // If new size is larger or only slightly smaller
  if (new_size > current_size || new_size > (current_size / 2)) {
    // Allocate new block
    void *new_ptr = dmalloc(new_size);
    if (__builtin_expect(new_ptr == NULL, 0)) {
      // If allocation fails, return original pointer
      return ptr;
    }
    
    // Copy data to new location (use smaller of two sizes)
    memcpy(new_ptr, ptr, (current_size < new_size) ? current_size : new_size);
    
    // Free old memory
    dfree(ptr);
    
    return new_ptr;
  }
  
  // Default case: return original pointer
  return ptr;
}

void dfree(void *ptr) {
  // Fast path: handle NULL pointer
  if (__builtin_expect(ptr == NULL, 0)) {
    return;
  }
  
#ifndef ONLY_SMALL
  // Fast path: determine allocation type
  void *page_start = calculate_page_start(ptr);
  AllocationType type = get_allocation_type(page_start);
  
  // Use switch with likely/unlikely hints for better branch prediction
  switch (type) {
    case BIN_ALLOCATION_TYPE:
      // Most common case for small allocations
      bin_free(ptr, page_start);
      break;
      
    case FREE_LIST_ALLOCATION_TYPE:
      // Medium allocations
      free_list_free(ptr, page_start);
      break;
      
    case HUGE_ALLOCATION_TYPE:
      // Large allocations
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
