#ifndef BIN_H
#define BIN_H

#include <stddef.h>
#include "allocator.h"
#include "bitset.h"

// A bin and all its metadata
typedef struct Bin {
  AllocationHeader header;
  // the memory where items are allocated
  void *ptr;
  // the next bin
  struct Bin *next;
  // the size of the objects allocated in the bin
  size_t bin_size;
  // the free spots in the bin
  BitSet bitset;
} Bin;

// This struct is responsible for managing a specifically sized bin
typedef struct {
  // the size of the blocks for this manages bins
  size_t bin_size;
  // the firt bin in the list
  Bin *head;
} BinManager;

// Allocates memory from one of the managers bins
// The caller must know the size of the bins in the manager
void *bin_manager_alloc(BinManager *manager);

// Frees memory from one of the managers bins
void bin_manager_free(void *ptr);

// Frees all memory from the bin manager
void bin_manager_free_all(BinManager *manager);

// Gets the size of the bin
// It requires a page aligned pointer
size_t bin_manager_size(BinManager *manager);

#endif
