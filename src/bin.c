#include "bin.h"
#include "allocator.h"
#include "bitset.h"
#include "mmap_allocator.h"
#include "page_store.h"
#include <stdio.h>

// A bin and all its metadata
typedef struct Bin {
  AllocationHeader header;
  // the mmap allocation for the bin
  MmapAllocation mmap_allocation;
  // the memory where items are allocated
  void *ptr;
  // the previous bin
  struct Bin *prev;
  // the next bin
  struct Bin *next;
  // the size of the objects allocated in the bin
  size_t bin_size;
  // Cache the number of free blocks for faster allocation decisions
  size_t free_blocks;
  // the free spots in the bin
  BitSet bitset;
} Bin;

// The bins to where memory can be allocated to
static Bin *bins[NUM_BINS] = {[0 ... NUM_BINS - 1] = NULL};

// Cache for recently used bins to improve locality
static Bin *recent_bins[NUM_BINS] = {[0 ... NUM_BINS - 1] = NULL};

// Precomputed bin sizes for faster lookup
static const size_t BIN_SIZES[NUM_BINS] = {1, 2, 4, 8, 16, 32, 64, 128};

// Precomputed lookup table for bin indices (for sizes 0-128)
static const unsigned char BIN_INDEX_LOOKUP[129] = {
    0, 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
};

// Checks if a bin is empty (no memory is allocated to it)
static inline bool is_bin_empty(Bin *bin) {
  return bin->free_blocks == bin->bitset.num_bits;
}

// Optimized bin index calculation with lookup table for common sizes
static inline size_t bin_index(size_t size) {
  // Fast path for most common small sizes using lookup table
  if (__builtin_expect(size <= 128, 1)) {
    return BIN_INDEX_LOOKUP[size];
  }

  // Handle edge case
  if (size <= 1)
    return 0;

  // Use built-in leading zero count for larger sizes
  return (sizeof(size_t) * 8) - __builtin_clzll(size - 1);
}

// Fast bin size lookup using precomputed table
static inline size_t calculate_bin_size(size_t index) {
  return BIN_SIZES[index];
}

// Calculates the number of bits needed for the bitset
static size_t calculate_bitset_size(size_t block_size) {
  // The amount of memory available (excluding the Bin structure)
  size_t total_memory_available = PAGE_SIZE - sizeof(Bin);

  // Initial estimate of total blocks
  size_t total_blocks = total_memory_available / block_size;

  // Initial estimate of bitset size
  size_t bitset_size = size_of_bitset(total_blocks);

  // Calculate the actual number of blocks after accounting for bitset size
  size_t available_for_blocks = total_memory_available - bitset_size;
  size_t new_total_blocks = available_for_blocks / block_size;

  // One more refinement is usually enough for convergence
  bitset_size = size_of_bitset(new_total_blocks);
  new_total_blocks = (total_memory_available - bitset_size) / block_size;

  return new_total_blocks;
}

static void init_bin(Bin *bin, size_t bin_size, MmapAllocation allocation) {
  // Set allocation type
  bin->header.allocation_type = BIN_ALLOCATION_TYPE;

  // Set mmap allocation
  bin->mmap_allocation = allocation;

  // Set bin size
  bin->bin_size = bin_size;

  // Initialize linked list pointers
  bin->prev = NULL;
  bin->next = NULL;

  // Calculate bitset size
  size_t num_bits = calculate_bitset_size(bin_size);

  // Initialize free block count
  bin->free_blocks = num_bits;

  // Initialize bitset
  init_bitset(&bin->bitset, num_bits);

  // Calculate pointer to the memory region for allocations
  size_t bitset_mem_size = size_of_bitset(num_bits);
  bin->ptr = (void *)((char *)bin + sizeof(Bin) + bitset_mem_size);
}

// Allocates memory to the passed in bin
static inline void *allocate_mem_to_bin(Bin *bin) {
  // Fast path: check if bin has free blocks
  if (__builtin_expect(bin->free_blocks == 0, 0)) {
    return NULL;
  }

  // Find first free available slot
  ssize_t index = find_first_unmarked_bit(&bin->bitset);

  // If no free slot found, return NULL
  if (__builtin_expect(index == -1, 0)) {
    bin->free_blocks = 0; // Update cache
    return NULL;
  }

  // Mark block as used
  mark_bit(&bin->bitset, index);
  bin->free_blocks--;

  // Calculate and return pointer to the allocated memory block
  return (void *)((char *)bin->ptr + index * bin->bin_size);
}

void *bin_alloc(size_t size) {
  // Fast path: determine bin index using optimized function
  size_t index = bin_index(size);

  // Try recent bin first for better cache locality
  Bin *recent = recent_bins[index];
  if (__builtin_expect(recent != NULL && recent->free_blocks > 0, 1)) {
    void *ptr = allocate_mem_to_bin(recent);
    if (__builtin_expect(ptr != NULL, 1)) {
      return ptr;
    }
  }

  // Get the head bin for this size
  Bin *current = bins[index];
  Bin *best_bin = NULL;
  size_t best_free_count = 0;

  // Search for a bin with available space
  if (current != NULL) {
    // Prefetch the bitset data to reduce cache misses
    __builtin_prefetch(&current->bitset, 0, 3);

    // Try up to 3 bins to balance search cost vs. fragmentation
    int bin_count = 0;
    while (current && bin_count < 3) {
      // Use cached free count for faster decision making
      if (current->free_blocks > 0 && current->free_blocks > best_free_count) {
        best_bin = current;
        best_free_count = current->free_blocks;

        // If we find a bin with lots of free space, use it immediately
        if (best_free_count > (current->bitset.num_bits / 4)) {
          break;
        }
      }

      // Prefetch the next bin
      if (current->next) {
        __builtin_prefetch(&current->next->bitset, 0, 3);
      }

      current = current->next;
      bin_count++;
    }

    // If we found a bin with free space, allocate from it
    if (best_bin) {
      void *ptr = allocate_mem_to_bin(best_bin);
      if (ptr) {
        // Update recent bin cache
        recent_bins[index] = best_bin;
        return ptr;
      }
    }
  }

  // No available bins or all bins are full, allocate a new one
  MmapAllocation allocation = retrieve_page();
  if (__builtin_expect(allocation.ptr == NULL, 0)) {
    return NULL; // Out of memory
  }

  // Initialize the new bin
  Bin *bin = (Bin *)allocation.ptr;
  size_t bin_size = calculate_bin_size(index);
  init_bin(bin, bin_size, allocation);

  // Insert at the head of the list
  bin->next = bins[index];
  bin->prev = NULL;
  if (bins[index] != NULL) {
    bins[index]->prev = bin;
  }
  bins[index] = bin;

  // Update recent bin cache
  recent_bins[index] = bin;

  // Allocate from the new bin
  return allocate_mem_to_bin(bin);
}

// Takes a pointer to memory to free as well as the bin which it belongs to
void bin_free(void *ptr, Bin *bin) {
  // Fast path: calculate index of the allocation
  size_t index = ((char *)ptr - (char *)bin->ptr) / bin->bin_size;

  // Unmark the bit in the bitset
  unmark_bit(&bin->bitset, index);
  bin->free_blocks++;

  // Check if bin is now empty
  if (__builtin_expect(is_bin_empty(bin), 0)) {
    size_t bin_index_val = bin_index(bin->bin_size);

    // Clear from recent cache if it's there
    if (recent_bins[bin_index_val] == bin) {
      recent_bins[bin_index_val] = bin->next;
    }

    // Remove bin from the linked list
    if (bin->prev == NULL) {
      // This bin is the head
      bins[bin_index_val] = bin->next;
      if (bin->next != NULL) {
        bin->next->prev = NULL;
      }
    } else {
      // This bin is in the middle or end of the list
      bin->prev->next = bin->next;
      if (bin->next != NULL) {
        bin->next->prev = bin->prev;
      }
    }

    // Return the page to the store
    store_page(bin->mmap_allocation);
  }
}

size_t bin_size(Bin *bin) { return bin->bin_size; }

Bin *allocated_by_bin(void *ptr) {
  // Calculate the page start address
  void *page_start = calculate_page_start(ptr);

  // Fast path: check if this is a bin allocation
  AllocationHeader *header = (AllocationHeader *)page_start;
  if (__builtin_expect(header->allocation_type == BIN_ALLOCATION_TYPE, 1)) {
    return (Bin *)page_start;
  }

  return NULL;
}

size_t num_bins() { return NUM_BINS; }
