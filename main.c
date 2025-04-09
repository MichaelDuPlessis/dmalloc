#include "src/allocator.h"
#include <stdio.h>

int main() {
  int *x = malloc(sizeof(int));
  *x = 42;

  printf("The answer to the life, the universe and everythign is %d\n", *x);

  free(x);
}
