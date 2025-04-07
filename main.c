#include <stdio.h>
#include "src/mmap_allocator.h"

int main() {
  int *x = mmap_malloc(sizeof(x));
  *x = 42;

  printf("The answer to life the universe and everything is %d\n", *x);

  mmap_free(x);
}
