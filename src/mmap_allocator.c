#include <unistd.h> 
#include <stdio.h>
#include <sys/mman.h>
#include "mmap_allocator.h"

void *mmap_malloc(size_t size) {
  // getting the page size
  size_t page_size = sysconf(_SC_PAGESIZE);
  // calculating how much memory will be allocated
  size_t alloc_size = ((size + page_size - 1) / page_size) * page_size;

  // allocating memory
  printf("Memory allocated: %zu\n", alloc_size);
  void* ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // storing how much memory was allocated
  size_t *header = (size_t*)ptr;
  *((size_t*)header) = alloc_size;

  // return memory just after the header
  return (void*)(header + 1);
}

void mmap_free(void *ptr) {
  // getting the header
  size_t *header = (size_t*)ptr - 1;
  size_t size = *header;

  printf("Memory deallocated: %zu\n", size);
  // deallocating memory
  munmap((void*)header, size);
}

