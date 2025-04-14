#ifndef BIN_H
#define BIN_H

#include <stddef.h>

// this type is used to represent the free spots in the region of memory
#define BITMASK unsigned long
// this is the max size of the type used to reprsent the bitmask
#define BITMASK_MAX (BITMASK)0 - 1

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
