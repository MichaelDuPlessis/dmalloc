#include "mmap_allocator.h"
#include <sys/mman.h>
#include "stdint.h"

// TODO: Sometimes it is easier to pass in the amount of memory that is needed
MmapAllocation mmap_alloc(size_t num_pages) {
  // getting the page size
  size_t page_size = PAGE_SIZE;
  // calculating how much memory will be allocated
  size_t alloc_size = num_pages * page_size;

  // allocating memory
  void *ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  return (MmapAllocation){
      .size = alloc_size,
      .ptr = ptr,
  };
}

void mmap_free(MmapAllocation alloc) {
  // deallocating memory
  munmap(alloc.ptr, alloc.size);
}

size_t calculate_num_pages(size_t size) {
  return (size + PAGE_SIZE - 1) / PAGE_SIZE;
}

void *calculate_page_start(void *ptr) {
  uintptr_t addr = (uintptr_t)ptr;
  return (void *)(addr & ~(PAGE_SIZE - 1));
}
