// This is the implementation of the huge allocator

#include "allocator.h"
#include "mmap_allocator.h"
#include <stddef.h>
#include "huge.h"

// This is the header for allocations from the huge allocator.
// It is assumed this header is smalled than a page size
// which it most likely will be
typedef struct {
  // what kind of allocation this is
  AllocationHeader header;
  // the size of the allocation, this is the amount of memory needed not used
  size_t size;
  // the mmap allocation details
  MmapAllocation mmap_allocation;
} HugeHeader;

// initializes the huge header
// from a mmap allocation
static inline void init_huge_header(HugeHeader *header, size_t size, MmapAllocation allocation) {
  *header = (HugeHeader){
    .header = HUGE_ALLOCATION_TYPE,
    .size = size,
    .mmap_allocation = allocation,
  };
}

void *huge_alloc(size_t size) {
  // figuring out how many pages are needed
  // since the allocation needs to also keep in mind
  // the sizeof the header
  size_t num_pages = calculate_num_pages(size + sizeof(HugeHeader));

  // allocating pages
  MmapAllocation allocation = mmap_alloc(num_pages);

  // creating header
  HugeHeader *header = (HugeHeader *)allocation.ptr;
  init_huge_header(header, size, allocation);

  // getting pointer to return
  void *ptr = (void *)(header + 1);

  return ptr;
}

void huge_free(void *ptr) {
  // getting start of page and extracting the header 
  HugeHeader *header = calculate_page_start(ptr);

  // deallocating memory using the mmap_allocation
  mmap_free(header->mmap_allocation);
}

size_t huge_size(void *ptr) {
  HugeHeader *header = (HugeHeader *)ptr;  
  return header->size;
}
