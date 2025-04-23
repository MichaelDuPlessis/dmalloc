#include "src/allocator.h"
#include <stdio.h>

#define TESTING_TYPE int

int main() {
  TESTING_TYPE *x = malloc(sizeof(TESTING_TYPE));
  TESTING_TYPE *y = malloc(sizeof(TESTING_TYPE));

  printf("Size of type is %lu.\n", sizeof(TESTING_TYPE));
  printf("Memory address of x: %p.\n", x);
  printf("Memory address of y: %p.\n", y);
  printf("Memory address of x: %lu.\n", (uintptr_t)x);
  printf("Memory address of y: %lu.\n", (uintptr_t)y);

  *x = 42;
  *y = 7;
  printf("The answer to the life, the universe and everything is %d.\n", *x);

  // free(x);
}
