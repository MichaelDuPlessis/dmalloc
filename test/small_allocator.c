#include "test.h"
#include <stdio.h>
#include "../src/allocator.h"
#include "assert.h"
#include "stdint.h"

#define TESTING_TYPE int

void small_allocator_basic_test() {
  printf("Size of type is %lu.\n", sizeof(TESTING_TYPE));

  TESTING_TYPE *x = dmalloc(sizeof(TESTING_TYPE));
  TESTING_TYPE *y = dmalloc(sizeof(TESTING_TYPE));

  printf("Memory address of x: %lu.\n", (uintptr_t)x);
  printf("Memory address of y: %lu.\n", (uintptr_t)y);

  assert(x + 1 == y);

  *x = 42;
  *y = 7;
  printf("The answer to the life, the universe and everything is %d.\n", *x);

  dfree(x);
  dfree(y);

  TESTING_TYPE *a = dmalloc(sizeof(TESTING_TYPE));
  TESTING_TYPE *b = dmalloc(sizeof(TESTING_TYPE));
  TESTING_TYPE *c = dmalloc(sizeof(TESTING_TYPE));

  assert(a == x);
  assert(b == y);

  printf("Memory address of a: %lu.\n", (uintptr_t)a);
  printf("Memory address of b: %lu.\n", (uintptr_t)b);
  printf("Memory address of c: %lu.\n", (uintptr_t)c);

  dfree(b);

  TESTING_TYPE *d = dmalloc(sizeof(TESTING_TYPE));
  printf("Memory address of d: %lu.\n", (uintptr_t)d);
  assert(d == b);

  dfree(a);
  dfree(c);
  dfree(d);
}
