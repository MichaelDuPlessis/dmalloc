#include "benchmark.h"
#include <stdio.h>
#include <time.h>

void basic_alloc(void *(*allocator)(size_t), void (*deallocator)(void *),
                 size_t amount, size_t alloc_size, const char *allocator_name) {
  void *allocations[amount];
  clock_t start, end;

  // Timing the first allocation cycle
  start = clock();
  for (int i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size); // Allocate memory for an integer
  }
  end = clock();
  double alloc_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the first deallocation cycle
  start = clock();
  for (int i = 0; i < amount; i++) {
    deallocator(allocations[i]); // Free the allocated memory
  }
  end = clock();
  double free_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the reallocation cycle (Allocate again after deallocation)
  start = clock();
  for (int i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size); // Reallocate memory
  }
  end = clock();
  double re_alloc_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the second deallocation cycle
  start = clock();
  for (int i = 0; i < amount; i++) {
    deallocator(allocations[i]); // Free the reallocated memory
  }
  end = clock();
  double re_free_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Print the results
  printf("Allocator: %s\n", allocator_name);
  printf("Time for %zu allocations: %.6f seconds\n", amount, alloc_time);
  printf("Time for %zu deallocations: %.6f seconds\n", amount, free_time);
  printf("Time for %zu reallocations: %.6f seconds\n", amount,
         re_alloc_time);
  printf("Time for %zu reallocations deallocations: %.6f seconds\n", amount,
         re_free_time);
}

