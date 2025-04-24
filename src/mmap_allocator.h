#ifndef MMAP_ALLOCATOR_H
#define MMAP_ALLOCATOR_H

#include <stddef.h>
#include <unistd.h>

// The size of the page on the os
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

// The memory from a mmap allocation
typedef struct {
  // The amount of memory allocated
  size_t size;
  // The pointer to the memory
  void* ptr;
} MmapAllocation;

// Allocates memory using mmap. It takes the number of pages to allocate
// and returns a struct containing a pointer to the allocated memory as well
// as the amount of memory allocated in bytes
MmapAllocation mmap_alloc(size_t size);

// Deallocates memory using mmap. It takes a struct containing the the pointer
// to the memory as well as the amount of memory to deallocate in bytes
void mmap_free(MmapAllocation alloc);

// This takes the amount of memory to allocate in bytesand returns the number
// of pages required for that amount of memory
size_t calculate_num_pages(size_t size);

#endif
