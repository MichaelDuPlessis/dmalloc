/*
  This file is used to benchmark various allocators.
  ALLOCATOR and DEALLOCATOR are still defined at compile time.
  Other configurations can now be passed via command-line arguments.
*/

#include "../src/allocator.h"
#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifndef ALLOCATOR
#define ALLOCATOR dmalloc
#endif
#ifndef DEALLOCATOR
#define DEALLOCATOR dfree
#endif
#ifndef NAME
#define NAME STR(ALLOCATOR)
#endif

typedef void (*BenchmarkFunc)(void *(*allocator)(size_t),
                                         void (*deallocator)(void *),
                                         size_t amount, size_t alloc_size,
                                         const char *allocator_name,
                                         unsigned int seed);

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr,
            "Usage: %s <benchmark_name> [amount] [size] [seed] [name]\n",
            argv[0]);
    return 1;
  }

  const char *benchmark_name = argv[1];
  size_t amount = (argc > 2) ? strtoull(argv[2], NULL, 10) : 10000;
  size_t size = (argc > 3) ? strtoull(argv[3], NULL, 10) : 1;
  size_t seed = (argc > 4) ? strtoull(argv[4], NULL, 10) : 42;
  const char *name = (argc > 5) ? argv[5] : NAME;

  BenchmarkFunc benchmark_fn = NULL;

  if (strcmp(benchmark_name, "basic") == 0) {
    benchmark_fn = basic_allocs;
  } else if (strcmp(benchmark_name, "sporadic") == 0) {
    benchmark_fn = sporadic_allocs;
  } else if (strcmp(benchmark_name, "varying") == 0) {
    benchmark_fn = varying_allocs;
  } else {
    fprintf(stderr, "Unknown benchmark: %s\n", benchmark_name);
    return 1;
  }

  return 0;
}
