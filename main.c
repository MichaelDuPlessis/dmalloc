// #include "src/allocator.h"
#include "benchmark/benchmark.h"
#include "src/allocator.h"
#include "test/test.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdalign.h>

int main() {
  // bitset_test();
  // small_allocator_basic_test();

  // printf("\nBenchmarking custom malloc/free:\n");
  // basic_alloc(malloc, free, 1000000, 4, "malloc/free");

  // printf("\nBenchmarking custom dmalloc/dfree:\n");
  // basic_alloc(dmalloc, dfree, 1000000, 4, "dmalloc/dfree");

  // printf("\nBenchmarking custom dmalloc/dfree:\n");
  // sporadic_alloc(dmalloc, dfree, 1000000, 4, "dmalloc/dfree");

  // printf("\nBenchmarking custom malloc/free:\n");
  // sporadic_alloc(malloc, free, 1000000, 4, "malloc/free");

  #define ALIGNMENT 8
  #define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
    printf("Alignment of int: %zu\n", alignof(int));
    printf("Alignment of double: %zu\n", alignof(double));
    printf("Alignment of void*: %zu\n", alignof(void *));
    printf("Alignment of long: %zu\n", alignof(long));
    printf("Alignment of size_t: %zu\n", alignof(size_t));
    printf("Alignment of size_t: %d\n", ALIGN(11));

  return 0;
}
