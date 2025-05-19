#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

BenchmarkResult basic_allocs(void *(*allocator)(size_t),
                             void (*deallocator)(void *), size_t amount,
                             size_t alloc_size, const char *allocator_name,
                             unsigned int seed) {
  void *allocations[amount];
  clock_t start, end;

  start = clock();

  // First allocation
  for (size_t i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size);
  }

  // First deallocation
  for (size_t i = 0; i < amount; i++) {
    deallocator(allocations[i]);
  }

  // Reallocation
  for (size_t i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size);
  }

  // Re-deallocation
  for (size_t i = 0; i < amount; i++) {
    deallocator(allocations[i]);
  }

  end = clock();

  BenchmarkResult result = {.allocator_name = allocator_name,
                            .benchmark_name = "basic",
                            .amount=amount,
                            .size=alloc_size,
                            .total_time =
                                (double)(end - start) / CLOCKS_PER_SEC};

  return result;
}
