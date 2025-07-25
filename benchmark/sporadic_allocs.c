#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>

void sporadic_allocs(void *(*allocator)(size_t), void (*deallocator)(void *),
                     size_t amount, size_t alloc_size, unsigned int seed) {
  void *allocations[amount];
  size_t allocated = 0;

  srand(seed);
  for (size_t i = 0; i < amount; i++) {
    allocations[i] = NULL;
  }

  for (size_t i = 0; i < amount * 2; i++) {
    int action = rand() % 2;

    if (action == 0 && allocated < amount) {
      size_t index;
      do {
        index = rand() % amount;
      } while (allocations[index] != NULL);
      allocations[index] = allocator(alloc_size);
      allocated++;
    } else if (action == 1 && allocated > 0) {
      size_t index;
      do {
        index = rand() % amount;
      } while (allocations[index] == NULL);
      deallocator(allocations[index]);
      allocations[index] = NULL;
      allocated--;
    }
  }

  for (size_t i = 0; i < amount; i++) {
    if (allocations[i] != NULL) {
      deallocator(allocations[i]);
    }
  }
}
