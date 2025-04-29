#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>


// allocates memory
void *dmalloc(size_t size);

// frees memory
void dfree(void *ptr);

#endif
