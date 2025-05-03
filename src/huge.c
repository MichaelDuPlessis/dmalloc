// This is the implementation of the huge allocator

#include "allocator.h"
#include "mmap_allocator.h"
#include "huge.h"

// This is the header for allocations from the huge allocator.
// It is assumed this header is smalled than a page size
// which it most likely will be
typedef struct {
  // what kind of allocation this is
  AllocationHeader header;
  // the mmap allocation details
  MmapAllocation mmap_allocation;
} HugeHeader;

// initializes the huge header
// from a mmap allocation
void init_huge_header(HugeHeader *header, MmapAllocation allocation) {
  *header = (HugeHeader){
    .header = HUGE_ALLOCATION_TYPE,
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
  init_huge_header(header, allocation);

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
