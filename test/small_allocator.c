#include "test.h"
#include <stdio.h>
#include "../src/allocator.h"

#define TESTING_TYPE int

void small_allocator_basic_test() {
  TESTING_TYPE *x = (TESTING_TYPE *)dmalloc(sizeof(TESTING_TYPE));
  // TESTING_TYPE *y = dmalloc(sizeof(TESTING_TYPE));

  printf("Size of type is %lu.\n", sizeof(TESTING_TYPE));
  printf("Memory address of x: %lu.\n", (uintptr_t)x);
  // printf("Memory address of y: %lu.\n", (uintptr_t)y);

  *x = 42;
  // *y = 7;
  printf("The answer to the life, the universe and everything is %d.\n", *x);

  dfree(x);
  // dfree(y);
}
