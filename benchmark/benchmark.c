#include "benchmark.h"
#include <stdio.h>

// Pretty-print to terminal
void print_result(BenchmarkResult result) {
  printf("Allocator: %s\n", result.allocator_name);
  printf("Benchmark: %s\n", result.benchmark_name);
  printf("Amount: %zu\n", result.amount);
  printf("Size: %zu\n", result.size);
  printf("Total time: %.6f seconds\n\n", result.total_time);
}

// Append result to a file (CSV-like)
int write_result_to_file(BenchmarkResult result, const char *filename) {
  FILE *file = fopen(filename, "a");
  if (!file) {
    perror("Error opening file");
    return -1;
  }

  // Write header if file is empty
  fseek(file, 0, SEEK_END);
  if (ftell(file) == 0) {
    fprintf(file, "allocator_name,benchmark_name,size,amount,total_time\n");
  }

  // Write result
  fprintf(file, "%s,%s,%zu,%zu,%f\n", result.allocator_name,
          result.benchmark_name, result.size, result.amount, result.total_time);

  fclose(file);
  return 0;
}
