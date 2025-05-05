#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void basic_alloc(void *(*allocator)(size_t), void (*deallocator)(void *),
                 size_t amount, size_t alloc_size, const char *allocator_name) {
  void *allocations[amount];
  clock_t start, end;

  // Timing the first allocation cycle
  start = clock();
  for (int i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size); // Allocate memory for an integer
  }
  end = clock();
  double alloc_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the first deallocation cycle
  start = clock();
  for (int i = 0; i < amount; i++) {
    deallocator(allocations[i]); // Free the allocated memory
  }
  end = clock();
  double free_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the reallocation cycle (Allocate again after deallocation)
  start = clock();
  for (int i = 0; i < amount; i++) {
    allocations[i] = allocator(alloc_size); // Reallocate memory
  }
  end = clock();
  double re_alloc_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Timing the second deallocation cycle
  start = clock();
  for (int i = 0; i < amount; i++) {
    deallocator(allocations[i]); // Free the reallocated memory
  }
  end = clock();
  double re_free_time = (double)(end - start) / CLOCKS_PER_SEC;

  // Print the results
  printf("Allocator: %s\n", allocator_name);
  printf("Time for %zu allocations: %.6f seconds\n", amount, alloc_time);
  printf("Time for %zu deallocations: %.6f seconds\n", amount, free_time);
  printf("Time for %zu reallocations: %.6f seconds\n", amount, re_alloc_time);
  printf("Time for %zu reallocations deallocations: %.6f seconds\n", amount,
         re_free_time);
}

void sporadic_alloc(void *(*allocator)(size_t), void (*deallocator)(void *),
                    size_t amount, size_t alloc_size,
                    const char *allocator_name) {
  void *allocations[amount];
  size_t allocated = 0;

  clock_t start, end;
  double total_alloc_time = 0.0, total_free_time = 0.0;

  srand((unsigned int)time(NULL)); // Seed for randomness

  // Initialize all to NULL
  for (size_t i = 0; i < amount; i++) {
    allocations[i] = NULL;
  }

  start = clock();
  for (size_t i = 0; i < amount * 2; i++) {
    int action = rand() % 2; // 0 = allocate, 1 = free

    if (action == 0 && allocated < amount) {
      // Allocate in a random empty slot
      size_t index;
      do {
        index = rand() % amount;
      } while (allocations[index] != NULL);

      clock_t t1 = clock();
      allocations[index] = allocator(alloc_size);
      clock_t t2 = clock();
      total_alloc_time += (double)(t2 - t1) / CLOCKS_PER_SEC;
      allocated++;
    } else if (action == 1 && allocated > 0) {
      // Deallocate a random non-empty slot
      size_t index;
      do {
        index = rand() % amount;
      } while (allocations[index] == NULL);

      clock_t t1 = clock();
      deallocator(allocations[index]);
      clock_t t2 = clock();
      total_free_time += (double)(t2 - t1) / CLOCKS_PER_SEC;
      allocations[index] = NULL;
      allocated--;
    }
  }
  end = clock();

  // Cleanup any remaining allocations
  clock_t cleanup_start = clock();
  for (size_t i = 0; i < amount; i++) {
    if (allocations[i] != NULL) {
      deallocator(allocations[i]);
      allocations[i] = NULL;
    }
  }
  clock_t cleanup_end = clock();
  double cleanup_time = (double)(cleanup_end - cleanup_start) / CLOCKS_PER_SEC;

  // Report results
  printf("Allocator: %s (sporadic)\n", allocator_name);
  printf("Total sporadic allocation time: %.6f seconds\n", total_alloc_time);
  printf("Total sporadic deallocation time: %.6f seconds\n", total_free_time);
  printf("Cleanup time for remaining allocations: %.6f seconds\n",
         cleanup_time);
}
