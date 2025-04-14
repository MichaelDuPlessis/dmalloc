#include "src/allocator.h"
#include <stdio.h>

int main() {
  int *x = malloc(sizeof(int));
  int *y = malloc(sizeof(int));

  printf("Size of int is %lu.\n", sizeof(int));
  printf("Memory address of x: %p.\n", x);
  printf("Memory address of y: %p.\n", y);

  *x = 42;
  *y = 7;
  printf("The answer to the life, the universe and everything is %d.\n", *x);

  // free(x);
}
