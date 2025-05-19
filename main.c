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
  BenchmarkResult res1 = basic_alloc(dmalloc, dfree, 10, sizeof(int) * 10, "dmalloc");
  BenchmarkResult res2 = sporadic_alloc(dmalloc, dfree, 10, sizeof(int) * 10, "dmalloc");

  print_result(res1);
  print_result(res2);
  return 0;
}
