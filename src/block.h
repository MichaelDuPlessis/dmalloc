#ifndef BLOCK_H
#define BLOCK_H

#include <stddef.h>

// the header of a block
typedef struct BlockHeader {
  // the amount of memory available
  size_t size;
} BlockHeader;

// Nodes in the free list
typedef struct BlockNode {
  // the next block in the list
  struct BlockNode *next;
  // the amount of memory used by the allocation
  size_t size;
} BlockNode;

#endif
