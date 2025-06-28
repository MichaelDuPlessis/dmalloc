#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>

void basic_allocs(void *(*allocator)(size_t), void (*deallocator)(void *),
                  size_t amount, size_t alloc_size, unsigned int seed) {
  void *allocations[amount];

  // First allocation
  for (size_t i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size);
  }


  // First deallocation
  for (size_t i = 0; i < amount; i++) {
    deallocator(allocations[i]);
  }

  return;

  // Reallocation
  for (size_t i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size);
  }

  // Re-deallocation
  for (size_t i = 0; i < amount; i++) {
    deallocator(allocations[i]);
  }
}
