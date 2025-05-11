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

  // printf("\nBenchmarking custom malloc/free:\n");
  // basic_alloc(malloc, free, 1000000, 4, "malloc/free");

  printf("\nBenchmarking custom dmalloc/dfree:\n");
  basic_alloc(dmalloc, dfree, 1000000, 4, "dmalloc/dfree");

  // printf("\nBenchmarking custom dmalloc/dfree:\n");
  // sporadic_alloc(dmalloc, dfree, 1000000, 4, "dmalloc/dfree");

  // printf("\nBenchmarking custom malloc/free:\n");
  // sporadic_alloc(malloc, free, 1000000, 4, "malloc/free");

  return 0;
}
