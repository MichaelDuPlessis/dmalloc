#include "error.h"
#include <stdio.h>
#include <stdlib.h>

// prints a message and aborts the program
void print_and_abort(const char* msg) {
  fprintf(stderr, "%s\n", msg);
  abort();
}

void out_of_memory_error() {
  print_and_abort("Error: System out of memory");
}

void invalid_free() {
  print_and_abort("Error: Invalid free");
}
