#include "bin.h"
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

void *bin_alloc(BinManager *bin_manager, size_t bin_size) {
  // finding the current bin with free memory
  Bin *current = bin_manager->head;

  // // finding the spot where to allocate the memory
  // size_t spot = find_first_free_bit(bin->mask);
  // // marking the spot as taken
  // bin->mask |= 1 << spot;
  // // returning the memory
  // return (void *)((char *)bin->ptr + spot);
}
