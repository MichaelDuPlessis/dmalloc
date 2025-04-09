#ifndef BLOCK_H
#define BLOCK_H

#include <stddef.h>

// the header of a block
typedef struct BlockHeader {
  // the amount of memory being used not including the header
  size_t size;
} BlockHeader;

// Nodes in the free list
typedef struct BlockNode {
  // the next spot for a block in the list
  struct BlockNode *next;
  // the amount of memory available to the block
  size_t size;
} BlockNode;

#endif
