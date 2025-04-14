#include "bin.h"
#include "distributer.h"
#include <stddef.h>
#include <stdio.h>

size_t find_first_free_bit(BITMASK bitmask) {
  // loop through each bit of the bitmask
  for (size_t i = 0; i < sizeof(bitmask) * 8; i++) {
    // check if the bit is free (0)
    if ((bitmask & (1 << i)) == 0) {
      return i; // return the index of the first free bit
    }
  }

  return -1; // return -1 if no free bits are found
}

// creates a bin of a specified size
Bin *create_bin(size_t size) {
  size_t bin_size = sizeof(Bin);

  // allocating memory to fit bins size
  // since the total amount of memory needed is the size of the bitmask + the
  // amount of objects which is the number of bits in the bitmask further
  // multiplied by the size of the objects that the bin must hold
  // TODO: Look into padding this for alignment
  Bin *bin = (Bin *)request_block(bin_size + sizeof(BITMASK) * 8 * size);
  *bin = (Bin) {
   .mask = 0,
   .next = NULL,
   .ptr = (void *)(bin + 1), 
  };

  return bin;
}

void *bin_alloc(BinManager *bin_manager, size_t bin_size) {
  // finding the current bin with free memory
  Bin *current = bin_manager->head;
  // if the current pointer is null it means we have reached the end of the list and therefore
  // must allocate more memory
  while (current && current->mask == BITMASK_MAX) {
    current = current->next;
  }

  // checking if current has memory associated with it
  if (!current) {
    Bin *bin = create_bin(bin_size);
    bin->next = bin_manager->head;
    bin_manager->head = bin;
    current = bin;
  }

  // finding the spot where to allocate the memory
  size_t spot = find_first_free_bit(current->mask);
  // marking the spot as taken
  current->mask |= 1 << spot;
  // returning the memory
  // this is done by moving from the pointer to the memory the index of where a free spot is
  // multiplied by how much space a spot takes up
  return (void *)((char *)current->ptr + spot * bin_size);
}
