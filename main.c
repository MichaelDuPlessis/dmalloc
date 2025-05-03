// #include "src/allocator.h"
#include "benchmark/benchmark.h"
#include "src/allocator.h"
#include "test/test.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  // bitset_test();
  // small_allocator_basic_test();

  // printf("Benchmarking standard malloc/free:\n");
  // benchmark(malloc, free, "malloc/free");

  printf("\nBenchmarking custom dmalloc/dfree:\n");
  basic_alloc(dmalloc, dfree, 1000000, 1, "dmalloc/dfree");

  return 0;
}
