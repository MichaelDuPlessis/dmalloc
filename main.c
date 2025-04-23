#include "src/allocator.h"
#include "src/mmap_allocator.h"
#include <stdio.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define TESTING_TYPE int

int main() {
  size_t page_size = sysconf(_SC_PAGESIZE);
  MmapAllocation m = mmap_alloc(1);
  printf("Size of page: %ld\n", page_size);
  printf("Memory address of m: %p.\n", m.ptr);
  printf("Memory address of m: %lu.\n", (uintptr_t)m.ptr);
  printf("Memory address divisable by page size: %d.\n", (uintptr_t)m.ptr % page_size == 0);
  mmap_free(m);
  // TESTING_TYPE *x = malloc(sizeof(TESTING_TYPE));
  // TESTING_TYPE *y = malloc(sizeof(TESTING_TYPE));

  // printf("Size of type is %lu.\n", sizeof(TESTING_TYPE));
  // printf("Memory address of x: %p.\n", x);
  // printf("Memory address of y: %p.\n", y);
  // printf("Memory address of x: %lu.\n", (uintptr_t)x);
  // printf("Memory address of y: %lu.\n", (uintptr_t)y);

  // *x = 42;
  // *y = 7;
  // printf("The answer to the life, the universe and everything is %d.\n", *x);

  // free(x);
}
