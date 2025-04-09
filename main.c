#include "src/allocator.h"
#include <stdio.h>

int main() {
  int *x = malloc(sizeof(int));

  free(x);
}
