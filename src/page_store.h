// This is used for keeping a cache of memory pages so that that can quickly be retrieved

#ifndef PAGE_STORE_H
#define PAGE_STORE_H

#include "mmap_allocator.h"

// Retrieves a stored page
MmapAllocation retrieve_page();

// Stores a page so that it can be retrieved later.
// If the store is full the page is deallocated instead
void store_page(MmapAllocation allocation);

#endif
