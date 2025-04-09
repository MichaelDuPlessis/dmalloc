#include "distributer.h"
#include <stddef.h>
#include <stdio.h>

void *malloc(size_t size) {
  return request_block(size);
}

void free(void *ptr) {
  printf("Freeing memory\n");
}
