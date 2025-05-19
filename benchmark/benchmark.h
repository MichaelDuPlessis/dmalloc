#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>

typedef struct {
  const char *allocator_name;
  const char *benchmark_name;
  double total_time;
} BenchmarkResult;

// Allocates the amount of objects specified, then deallocates them, then reallocates them and then deallocates them
BenchmarkResult basic_alloc(void *(*allocator)(size_t),
                            void (*deallocator)(void *), size_t amount,
                            size_t alloc_size, const char *allocator_name);

// Randomly allocates and deallocates the amount of times specified and then deallocates everything
BenchmarkResult sporadic_alloc(void *(*allocator)(size_t),
                               void (*deallocator)(void *), size_t amount,
                               size_t alloc_size, const char *allocator_name);

// Prints a Benchmark result to stdout
void print_result(BenchmarkResult result);

// Writes a BenchmarkResult to a file
int write_result_to_file(BenchmarkResult result, const char *filename);

#endif
