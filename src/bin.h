#ifndef BIN_H
#define BIN_H

#include <stddef.h>

// this type is used to represent 8 bytes
#define BITMASK unsigned long

// A bin and all its metadata
typedef struct Bin {
  // the free spots in the bin
  BITMASK mask;
  // the memory where items are allocated
  void *ptr;
  // the next bin
  struct Bin *next;
} Bin;

// this struct is responsible for managing a specifically sized bin
typedef struct {
  // the firt bin in the list
  Bin *head;
} BinManager;

// allocates memory from a bin
void *bin_alloc(BinManager *bin_manager, size_t bin_size);

#endif
