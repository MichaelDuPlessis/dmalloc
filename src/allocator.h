#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>


// allocates memory
void *malloc(size_t size);

// frees memory
void free(void *ptr);

#endif
