#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>

// Allocates the amount of objects specified, then deallocates them, then
// reallocates them and then deallocates them
void basic_allocs(void *(*allocator)(size_t),
                             void (*deallocator)(void *), size_t amount,
                             size_t alloc_size, const char *allocator_name,
                             unsigned int seed);

// Randomly allocates and deallocates the amount of times specified and then
// deallocates everything
void sporadic_allocs(void *(*allocator)(size_t),
                                void (*deallocator)(void *), size_t amount,
                                size_t alloc_size, const char *allocator_name,
                                unsigned int seed);

// Does varying allocations of varying sizes
void varying_allocs(void *(*allocator)(size_t),
                               void (*deallocator)(void *), size_t amount,
                               size_t size, const char *allocator_name,
                               unsigned int seed);
#endif
