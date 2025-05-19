#include "benchmark.h"
#include <stdio.h>

// Pretty-print to terminal
void print_result(BenchmarkResult result) {
  printf("Allocator: %s\n", result.allocator_name);
  printf("Benchmark: %s\n", result.benchmark_name);
  printf("Amount: %zu\n", result.amount);
  printf("Total time: %.6f seconds\n\n", result.total_time);
}

// Append result to a file (CSV-like)
int write_results_to_file(BenchmarkResult *results, size_t amount,
                          const char *filename) {
  FILE *file = fopen(filename, "a");
  if (!file) {
    perror("Error opening file");
    return -1;
  }

  // Write header if file is empty
  fseek(file, 0, SEEK_END);
  if (ftell(file) == 0) {
    fprintf(file, "allocator_name,benchmark_name,amount,total_time\n");
  }

  for (size_t i = 0; i < amount; i++) {
    BenchmarkResult result = results[i];
    // Write result
    fprintf(file, "%s,%s,%zu,%.6f\n", result.allocator_name, result.benchmark_name,
            result.amount, result.total_time);
  }

  fclose(file);
  return 0;
}
