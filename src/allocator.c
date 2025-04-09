#include <stddef.h>
#include <stdio.h>

void *malloc(size_t size) {
  printf("Allocating memory\n");
}

void free(void *ptr) {
  printf("Freeing memory\n");
}
