#ifndef MMAP_ALLOCATOR_H
#define MMAP_ALLOCATOR_H

#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>

// The size of the page on the os
// #define PAGE_SIZE sysconf(_SC_PAGESIZE)

// Gets the page size of the os.
// This function caches the pagesize to avoid system calls
// TODO: Check if this should be inlined
size_t get_page_size();

#define PAGE_SIZE (get_page_size())

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
MmapAllocation mmap_alloc(size_t num_pages);

// Deallocates memory using mmap. It takes a struct containing the the pointer
// to the memory as well as the amount of memory to deallocate in bytes
void mmap_free(MmapAllocation alloc);

// This takes the amount of memory to allocate in bytesand returns the number
// of pages required for that amount of memory
size_t calculate_num_pages(size_t size);

// Giving a memory adderess that exist in a region of memory of a page
// it returns the start address of the page. The page allocations must
// be allocated in powers of 2 for this function to work
void *calculate_page_start(void* ptr);

// Prints the size and ptr of the MmapAllocation
void print_mmap_allocation(const MmapAllocation *allocation);

// Determines whether a ptr lies within an MmapAllocation
bool mmap_contains_ptr(const MmapAllocation allocation, char *ptr);

#endif
