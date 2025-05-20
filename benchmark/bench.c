/*
  This file is used to benchmark various allocators. Currently the code has to be recompiled with different macros being defined at compilation time.
  This should change to make use of command line arguments as well as MACROS defined at compilation time.
*/

#include "../src/allocator.h"
#include <stdlib.h>
#include "benchmark.h"
#include <stdio.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// the benchmark to use
#ifndef BENCHMARK
#define BENCHMARK varying_allocs
#endif

// the amount timen s to allocate memory
#ifndef AMOUNT
#define AMOUNT 10000
#endif

// the seed to use
#ifndef SEED
#define SEED 42
#endif

// the allocator to use
#ifndef ALLOCATOR
#define ALLOCATOR dmalloc
#endif
#ifndef DEALLOCATOR
#define DEALLOCATOR dfree
#endif

// the allocators name
#ifndef NAME
#define NAME STR(ALLOCATOR)
#endif

// the size of the allocations to make
#ifndef SIZE
#define SIZE 0
#endif

// the amount of steps to use
// the allocator is run the amount of times equal to the value of STEPS
// the amount of items to allocate increases as the steps increase
#ifndef STEPS
#define STEPS 10
#endif

int main() {
  // after that the tests can be run
  BenchmarkResult result = BENCHMARK(ALLOCATOR, DEALLOCATOR, AMOUNT, SIZE, NAME, SEED);

  // the actual benchmark
#ifdef PRINT_RES
    print_result(result);
#endif

  // Create dynamic filename based on ALLOCATOR and BENCHMARK
  char filename[256];
  snprintf(filename, sizeof(filename), "./results/%s_%s_%zu.csv", NAME, STR(BENCHMARK), (size_t)SIZE);
  write_result_to_file(result, filename);

  return 0;
}
