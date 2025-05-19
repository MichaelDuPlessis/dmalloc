#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>

typedef struct {
  // The name of the allocator
  const char *allocator_name;
  // The name of the benchmark
  const char *benchmark_name;
  // The amount of objects allocated
  size_t amount;
  // The total amount of time
  double total_time;
} BenchmarkResult;

// Allocates the amount of objects specified, then deallocates them, then
// reallocates them and then deallocates them
BenchmarkResult basic_allocs(void *(*allocator)(size_t),
                             void (*deallocator)(void *), size_t amount,
                             size_t alloc_size, const char *allocator_name, unsigned int seed);

// Randomly allocates and deallocates the amount of times specified and then
// deallocates everything
BenchmarkResult sporadic_allocs(void *(*allocator)(size_t),
                                void (*deallocator)(void *), size_t amount,
                                size_t alloc_size, const char *allocator_name,
                                unsigned int seed);

// Does varying allocations of varying sizes
BenchmarkResult varying_allocs(void *(*allocator)(size_t),
                               void (*deallocator)(void *), size_t amount,
                               size_t size,
                               const char *allocator_name, unsigned int seed);

// Prints a Benchmark result to stdout
void print_result(BenchmarkResult result);

// Writes a BenchmarkResult to a file
int write_results_to_file(BenchmarkResult *results, size_t amount, const char *filename);

#endif
