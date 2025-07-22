# dmalloc Optimization TODO List

This document outlines potential further optimizations for the dmalloc memory allocator that have been identified but not yet implemented.

## Implemented Optimizations
- ✅ Optimized bin index calculation using bit manipulation
- ✅ Improved bitset operations for better performance
- ✅ Enhanced bin allocation strategy to reduce fragmentation
- ✅ Added prefetching to reduce cache misses
- ✅ Added compiler optimization attributes

## Remaining Optimization Opportunities

### 1. SIMD Bitset Operations
Use SIMD (Single Instruction Multiple Data) instructions to process multiple words in the bitset simultaneously:

```c
// Find first unmarked bit using SIMD instructions
ssize_t find_first_unmarked_bit(BitSet *bitset) {
  // Process 4 words at a time using SIMD
  for (; word_idx + 4 <= num_words; word_idx += 4) {
    // Use SIMD intrinsics to check 4 words at once
    #ifdef __AVX2__
      __m256i words = _mm256_loadu_si256((__m256i*)&bitset->words[word_idx]);
      __m256i all_ones = _mm256_set1_epi64x(MAX_WORD_SIZE);
      int mask = _mm256_movemask_pd(_mm256_castsi256_pd(_mm256_cmpeq_epi64(words, all_ones)));
      
      if (mask != 0xF) { // At least one word has free bits
        // Find which word has free bits
        int free_word_offset = __builtin_ctz(~mask & 0xF);
        WORD word = bitset->words[word_idx + free_word_offset];
        WORD inverted_word = ~word;
        unsigned int bit_pos = __builtin_ctzll(inverted_word);
        
        bitset->free_word_index = word_idx + free_word_offset;
        return (ssize_t)((word_idx + free_word_offset) * BITS_PER_WORD + bit_pos);
      }
    #endif
  }
}
```

### 2. Thread-Local Caching
For multi-threaded applications, implement thread-local caching to reduce contention:

```c
// Thread-local cache of recently freed small objects
__thread void* small_object_cache[NUM_BINS][CACHE_SIZE];
__thread int cache_count[NUM_BINS];

void* dmalloc(size_t size) {
  // Check thread-local cache first for small allocations
  if (size <= MAX_BIN_SIZE) {
    size_t bin_idx = bin_index(size);
    if (cache_count[bin_idx] > 0) {
      return small_object_cache[bin_idx][--cache_count[bin_idx]];
    }
  }
  
  // Existing allocation code...
}
```

### 4. Bin Coalescing
Implement a strategy to coalesce partially filled bins to reduce fragmentation:

```c
void coalesce_bins(size_t bin_idx) {
  // Only coalesce if we have multiple bins for this size
  if (bins[bin_idx] == NULL || bins[bin_idx]->next == NULL) {
    return;
  }
  
  // Find bins that are less than half full
  Bin* candidates[MAX_COALESCE_CANDIDATES];
  int candidate_count = 0;
  
  Bin* current = bins[bin_idx];
  while (current && candidate_count < MAX_COALESCE_CANDIDATES) {
    size_t usage = current->bitset.num_bits_marked * 100 / current->bitset.num_bits;
    if (usage < 50) {
      candidates[candidate_count++] = current;
    }
    current = current->next;
  }
  
  // If we have enough candidates, coalesce them
  if (candidate_count >= 2) {
    // Move all allocations from second bin to first bin
    // Then free the second bin
  }
}
```

### 5. Power-of-Two Size Classes Optimization
Optimize the bin sizes to better match common allocation patterns:

```c
// Define more granular size classes for small allocations
#define NUM_SMALL_BINS 4
#define NUM_MEDIUM_BINS 4

static const size_t small_bin_sizes[NUM_SMALL_BINS] = {8, 16, 32, 64};
static const size_t medium_bin_sizes[NUM_MEDIUM_BINS] = {128, 256, 512, 1024};
```

### 6. Memory Alignment Optimization
Ensure proper memory alignment for better performance:

```c
void* bin_alloc(size_t size) {
  // Calculate aligned size
  size_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
  
  // Rest of allocation code using aligned_size...
}
```

### 8. Specialized Allocators for Common Sizes
Add specialized fast paths for the most common allocation sizes:

```c
void* dmalloc(size_t size) {
  // Fast paths for extremely common sizes
  if (size == 16) return size16_alloc();
  if (size == 32) return size32_alloc();
  if (size == 64) return size64_alloc();
  
  // Existing allocation code...
}
```

### 9. Reduce Metadata Overhead
Optimize the Bin structure to reduce its size and improve cache efficiency:

```c
typedef struct Bin {
  AllocationHeader header;
  // Use a more compact representation
  uint32_t bin_size;  // Instead of size_t
  uint16_t prev_offset; // Store offset instead of pointer
  uint16_t next_offset; // Store offset instead of pointer
  // Rest of the structure...
} Bin;
```

### 10. Batch Processing
Implement batch allocation and deallocation for better performance:

```c
void* batch_alloc(size_t size, size_t count, void** results) {
  // Allocate multiple objects of the same size at once
  // This can be more efficient than individual allocations
}
```

## Performance Profiling
Before implementing any of these optimizations, it would be beneficial to:

1. Run a detailed profiler (like perf, gprof, or Valgrind) to identify remaining hotspots
2. Analyze cache miss rates and branch mispredictions
3. Test with different workloads to ensure optimizations are generally beneficial

## Implementation Priority
Based on expected impact and implementation complexity:

1. Memory Alignment Optimization (high impact, low complexity)
2. Specialized Allocators for Common Sizes (high impact, medium complexity)
3. SIMD Bitset Operations (high impact, high complexity)
4. Reduce Metadata Overhead (medium impact, medium complexity)
5. Bin Coalescing (medium impact, high complexity)
