# dmalloc

A memory allocator optimized for allocating many small objects in data science applications.

## Project Structure

```
dmalloc/
├── src/                     # Core allocator implementation
│   ├── allocator.*          # Main memory allocation entry point
│   ├── bin.*                # Bin allocator implementation
│   ├── bitset.*             # Bitset data structure
│   ├── error.*              # Error handling utilities
│   ├── free_list.*          # Free list allocator implementation
│   ├── huge.*               # Page allocator for large objects
│   ├── mmap_allocator.*     # Wrapper around mmap syscall
│   └── page_store.*         # Memory page cache
├── benchmark/               # Benchmarking implementations
├── benchmark_time.sh        # Time performance benchmarks
├── benchmark_mem.sh         # Memory usage benchmarks
└── justfile                 # Build automation
```


