#include "src/allocator.h"
#include "benchmark/benchmark.h"

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

int main() {
  BenchmarkResult res = BENCHMARK(ALLOCATOR, DEALLOCATOR, AMOUNT, SIZE, NAME, SEED);

  print_result(res);

  return 0;
}
