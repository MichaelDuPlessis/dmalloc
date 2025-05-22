#ifndef BIN_H
#define BIN_H

#include <stddef.h>
#include "allocator.h"
#include "bitset.h"

// forward declaration
struct Bin;

// This struct is responsible for managing a specifically sized bin
typedef struct {
  // the size of the blocks for this manages bins
  size_t bin_size;
  // the firt bin in the list
  struct Bin *head;
} BinManager;

// Allocates memory from one of the managers bins
// The caller must know the size of the bins in the manager
void *bin_manager_alloc(BinManager *manager);

// Frees memory from one of the managers bins
// It requires a pointer to the memory to be free as well as
// the pointer to the bin
void bin_manager_free(void *ptr, struct Bin *bin);

// Frees all memory from the bin manager
void bin_manager_free_all(BinManager *manager);

// Gets the size of the bin
// It requires a page aligned pointer
size_t bin_manager_size(BinManager *manager);

#endif
