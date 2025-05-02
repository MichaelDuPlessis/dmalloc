// #include "src/allocator.h"
#include "src/allocator.h"
#include "test/test.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

void benchmark(void *(*allocator)(size_t), void (*deallocator)(void *),
  #define TYPE int
               
               const char *allocator_name) {
  const int ALLOCATIONS = 1000000; // Number of allocations and deallocations
  TYPE *allocations[ALLOCATIONS];
  clock_t start, end;

  // Timing the first allocation cycle
  start = clock();
  for (int i = 0; i < ALLOCATIONS; i++) {
    allocations[i] = allocator(sizeof(TYPE)); // Allocate memory for an integer
  }
  end = clock();
  double alloc_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the first deallocation cycle
  start = clock();
  for (int i = 0; i < ALLOCATIONS; i++) {
    deallocator(allocations[i]); // Free the allocated memory
  }
  end = clock();
  double free_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the reallocation cycle (Allocate again after deallocation)
  start = clock();
  for (int i = 0; i < ALLOCATIONS; i++) {
    allocations[i] = allocator(sizeof(TYPE)); // Reallocate memory for an integer
  }
  end = clock();
  double re_alloc_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the second deallocation cycle
  start = clock();
  for (int i = 0; i < ALLOCATIONS; i++) {
    deallocator(allocations[i]); // Free the reallocated memory
  }
  end = clock();
  double re_free_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Print the results
  printf("Allocator: %s\n", allocator_name);
  printf("Time for %d allocations: %.6f seconds\n", ALLOCATIONS, alloc_time);
  printf("Time for %d deallocations: %.6f seconds\n", ALLOCATIONS, free_time);
  printf("Time for %d reallocations: %.6f seconds\n", ALLOCATIONS,
         re_alloc_time);
  printf("Time for %d reallocations deallocations: %.6f seconds\n", ALLOCATIONS,
         re_free_time);
}

int main() {
  // bitset_test();
  // small_allocator_basic_test();

  // printf("Benchmarking standard malloc/free:\n");
  // benchmark(malloc, free, "malloc/free");

  printf("\nBenchmarking custom dmalloc/dfree:\n");
  benchmark(dmalloc, dfree, "dmalloc/dfree");

  return 0;
}
