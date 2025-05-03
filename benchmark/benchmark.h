// All the functions for benchmarking

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>

// Allocates n objects of a certain s size an then deallocates all objects, reallocates them and then deallocates them again.
// It also prints the allocators name passed in
void basic_alloc(void *(*allocator)(size_t), void (*deallocator)(void *), size_t amount, size_t alloc_size, const char *allocator_name);

#endif
