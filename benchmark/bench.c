#include "../src/allocator.h"
#include "benchmark.h"

// the benchmark to use
#ifndef BENCHMARK
#define BENCHMARK basic_allocs
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
#define NAME "dmalloc"
#endif

// the size of the allocations to make
#ifndef SIZE
#define SIZE sizeof(int)
#endif

// the amount of steps to use
// the allocator is run the amount of times equal to the value of STEPS
// the amount of items to allocate increases as the steps increase
#ifndef STEPS
#define STEPS 10
#endif

int main() {
  // first warm up the code
  BenchmarkResult res = basic_allocs(ALLOCATOR, DEALLOCATOR, 10, SIZE, NAME, SEED);

  // after that the tests can be run
  BenchmarkResult results[STEPS];

  // the actual benchmark
  size_t step_size = AMOUNT / STEPS;
  for (size_t i = 0; i < STEPS; i++) {
    results[i] = BENCHMARK(ALLOCATOR, DEALLOCATOR, step_size + i * step_size, SIZE, NAME, SEED);

#ifdef PRINT_RES
    print_result(results[i]);
#endif
  }

  write_results_to_file(results, STEPS, "./results/results.csv");

  return 0;
}
