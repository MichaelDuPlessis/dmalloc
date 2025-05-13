#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stddef.h>

// Allocates memory to the free list.
// It uses a first fit algorithm
void *free_list_alloc(size_t size);

// Deallocates memory from the free list
void free_list_free(void *ptr);

#endif
