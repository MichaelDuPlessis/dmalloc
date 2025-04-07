#ifndef MMAP_ALLOCATOR_H
#define MMAP_ALLOCATOR_H

#include <stddef.h>

void* mmap_malloc(size_t size);

void mmap_free(void *ptr);

#endif
