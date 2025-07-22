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
  // the free spots in the bin
  BitSet bitset;
} Bin;

// The bins to where memory can be allocated to
static Bin *bins[NUM_BINS] = {[0 ... NUM_BINS - 1] = NULL};

// Checks if a bin is empty (no memory is allocated to it)
static inline bool is_bin_empty(Bin *bin) {
  return all_bits_unmarked(&bin->bitset);
}

// Finding which bin an allocation belongs to
static inline size_t bin_index(size_t size) {
  // Handle edge case
  if (size <= 1) return 0;
  
  // Use built-in leading zero count for faster power-of-2 ceiling calculation
  // size-1 ensures we round up to the next power of 2 for non-power-of-2 sizes
  return (sizeof(size_t) * 8) - __builtin_clzll(size - 1);
}

// Calculates the number of blocks that can fit in some amount of memory
static inline size_t calculate_num_blocks(size_t block_size,
                                          size_t total_memory) {
  return total_memory / block_size;
}

// Calculates the size of a bin based off of the index it
// belongs
static inline size_t calculate_bin_size(size_t index) {
  // Since bins are always powers of 2 we just have to shift
  // by the index used
  size_t bin_size = 1 << index;
  return bin_size;
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
  // Use a more direct calculation to avoid iterations
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
  
  // Calculate bitset size (already optimized)
  size_t num_bits = calculate_bitset_size(bin_size);
  
  // Initialize bitset
  init_bitset(&bin->bitset, num_bits);
  
  // Calculate pointer to the memory region for allocations
  // This avoids an extra calculation in the hot path
  size_t bitset_mem_size = size_of_bitset(num_bits);
  bin->ptr = (void *)((char *)bin + sizeof(Bin) + bitset_mem_size);
}

// Allocates memory to the passed in bin
static void *allocate_mem_to_bin(Bin *bin) {
  // Fast path: find first free available slot
  ssize_t index = find_first_unmarked_bit(&bin->bitset);
  
  // If no free slot found, return NULL immediately
  if (__builtin_expect(index == -1, 0)) {
    return NULL;
  }
  
  // Mark block as used (already optimized in mark_bit)
  mark_bit(&bin->bitset, index);
  
  // Calculate and return pointer to the allocated memory block
  // Use direct pointer arithmetic for better performance
  return (void *)((char *)bin->ptr + index * bin->bin_size);
}

void *bin_alloc(size_t size) {
  // Fast path: determine bin index using optimized function
  size_t index = bin_index(size);
  
  // Get the head bin for this size
  Bin *current = bins[index];
  Bin *best_bin = NULL;
  size_t best_free_count = 0;
  
  // Fast path: if we have a bin with this size, try to allocate from it
  if (current != NULL) {
    // Prefetch the bitset data to reduce cache misses
    __builtin_prefetch(&current->bitset, 0, 3);
    
    // First try the head bin (most common case)
    void *ptr = allocate_mem_to_bin(current);
    if (ptr) {
      return ptr;
    }
    
    // If first bin is full, search for a bin with the most free space
    // This helps reduce fragmentation by filling up bins more completely
    current = current->next;
    
    // Prefetch the next bin if it exists
    if (current) {
      __builtin_prefetch(&current->bitset, 0, 3);
    }
    
    // Only check up to 3 bins to avoid excessive searching
    int bin_count = 0;
    while (current && bin_count < 3) {
      // Count free bits in this bin
      size_t free_count = current->bitset.num_bits - current->bitset.num_bits_marked;
      
      // If this bin has free space and more than our current best, use it
      if (free_count > 0 && free_count > best_free_count) {
        best_bin = current;
        best_free_count = free_count;
      }
      
      // Prefetch the next bin in the list to reduce cache misses
      if (current->next) {
        __builtin_prefetch(&current->next->bitset, 0, 3);
      }
      
      current = current->next;
      bin_count++;
    }
    
    // If we found a bin with free space, allocate from it
    if (best_bin) {
      ptr = allocate_mem_to_bin(best_bin);
      if (ptr) {
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

  // Insert at the head of the list for better cache locality on future allocations
  bin->next = bins[index];
  bin->prev = NULL;
  if (bins[index] != NULL) {
    bins[index]->prev = bin;
  }
  bins[index] = bin;

  // Allocate from the new bin
  return allocate_mem_to_bin(bin);
}

// Takes a pointer to memory to free as well as the bin which it belongs to
void bin_free(void *ptr, Bin *bin) {
  // Fast path: calculate index of the allocation
  // (allocation_ptr - start_ptr) / bin_size = index
  size_t index = ((char *)ptr - (char *)bin->ptr) / bin->bin_size;
  
  // Unmark the bit in the bitset (already optimized)
  unmark_bit(&bin->bitset, index);
  
  // Check if bin is now empty - only free the bin if it's completely empty
  // to reduce fragmentation and allocation overhead
  if (__builtin_expect(is_bin_empty(bin), 0)) {
    size_t bin_index_val = bin_index(bin->bin_size);
    
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

// void bin_manager_free_all(BinManager *manager) {
//   Bin *current = manager->head;
//   while (current) {
//     Bin *next = current->next;

//     // deallocating memory
//     MmapAllocation allocation = {
//         .ptr = (void *)current,
//         .size = 1,
//     };
//     mmap_free(allocation);

//     current = next;
//   }

//   manager->head = NULL;
// }

size_t bin_size(Bin *bin) { return bin->bin_size; }

Bin *allocated_by_bin(void *ptr) {
  // Calculate the page start address for faster comparison
  void *page_start = calculate_page_start(ptr);
  
  // First check if the pointer is page-aligned, which would indicate
  // it's the start of a bin (most common case for bin allocations)
  if (page_start == ptr) {
    // Check if this page start is a bin by examining its header
    AllocationHeader *header = (AllocationHeader *)page_start;
    if (header->allocation_type == BIN_ALLOCATION_TYPE) {
      return (Bin *)page_start;
    }
  }
  
  // If not found via direct check, search through all bins
  // Start with smaller bins as they're more common
  for (size_t i = 0; i < NUM_BINS; i++) {
    Bin *current = bins[i];
    
    while (current) {
      // Check if the pointer is within this bin's memory range
      if (mmap_contains_ptr(current->mmap_allocation, ptr)) {
        return current;
      }
      current = current->next;
    }
  }
  
  return NULL;
}
