#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stddef.h>

struct Chunk;

// Allocates memory to the free list.
// It uses a first fit algorithm
void *free_list_alloc(size_t size);

// Deallocates memory from the free list
// It requires the chunk to which the pointer belongs to be passed in alsoo
void free_list_free(void *ptr, struct Chunk *chunk);

// Returns the size of the memory allocated for that object in the free list
size_t free_list_size(void *ptr);

#endif
