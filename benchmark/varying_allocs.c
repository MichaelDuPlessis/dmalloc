#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

BenchmarkResult varying_allocs(void *(*allocator)(size_t),
                               void (*deallocator)(void *), size_t amount,
                               size_t alloc_size, const char *allocator_name,
                               unsigned int seed) {
  void **allocations = (void **)allocator(amount * sizeof(void *));
  size_t *sizes = (size_t *)allocator(amount * sizeof(size_t));
  if (!allocations || !sizes) {
    fprintf(stderr, "Memory allocation for benchmark failed\n");
    exit(EXIT_FAILURE);
  }

  srand(seed); // Set seed for reproducibility

  clock_t start = clock();

  // Allocate varying sizes
  for (size_t i = 0; i < amount; ++i) {
    size_t size = 1 + rand() % 4081; // [1, 4096]
    sizes[i] = size;
    allocations[i] = allocator(size);
    if (!allocations[i]) {
      fprintf(stderr, "Allocation %zu failed for size %zu\n", i, size);
    }
  }

  // Shuffle allocation pointers
  for (size_t i = 0; i < amount - 1; ++i) {
    size_t j = i + rand() % (amount - i);
    void *tmp = allocations[i];
    allocations[i] = allocations[j];
    allocations[j] = tmp;
  }

  // Deallocate
  for (size_t i = 0; i < amount; ++i) {
    if (allocations[i]) {
      deallocator(allocations[i]);
    }
  }

  clock_t end = clock();
  double total_time = (double)(end - start) / CLOCKS_PER_SEC;

  deallocator(allocations);
  deallocator(sizes);

  BenchmarkResult result = {.allocator_name = allocator_name,
                            .benchmark_name = "varying_allocs",
                            .amount=amount,
                            .size=alloc_size,
                            .total_time = total_time};
  return result;
}
