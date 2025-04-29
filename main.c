// #include "src/allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include "test/test.h"


int main() {
  // bitset_test();
  small_allocator_basic_test();

  return 0;
}
