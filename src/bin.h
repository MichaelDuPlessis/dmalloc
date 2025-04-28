#ifndef BIN_H
#define BIN_H

#include <stddef.h>
#include "bitset.h"

// A bin and all its metadata
typedef struct Bin {
  // the memory where items are allocated
  void *ptr;
  // the next bin
  struct Bin *next;
  // the size of the objects allocated in the bin
  size_t bin_size;
  // the free spots in the bin
  BitSet bitset;
} Bin;

// this struct is responsible for managing a specifically sized bin
typedef struct {
  // the firt bin in the list
  Bin *head;
} BinManager;

// initializes the bin
void init_bin(Bin *bin, size_t bin_size);

#endif
