#ifndef MMAP_ALLOCATOR_H
#define MMAP_ALLOCATOR_H

#include <stddef.h>
#include <unistd.h>

// the size of the page on the os
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

// The memory from a mmap allocation
typedef struct {
  // the amount of memory allocated
  size_t size;
  // the pointer to the memory
  void* ptr;
} MmapAllocation;

// allocates memory using mmap. It takes the number of pages to allocate
MmapAllocation mmap_alloc(size_t size);

// deallocates memory using mmap
void mmap_free(MmapAllocation alloc);

#endif
