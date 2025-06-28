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
  // TODO: maybe don't use size_t it may not be necessary
  size_t bin = 0;
  size_t power = 1;
  while (power < size) {
    power <<= 1;
    bin++;
  }
  return bin;
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
  // the amount of memory availble
  // since I don't want to include th bitsets values
  size_t total_memory_available = PAGE_SIZE - (sizeof(Bin) - sizeof(BitSet));

  size_t total_blocks =
      calculate_num_blocks(block_size, total_memory_available);
  // size of bitset required
  size_t bitset_size = size_of_bitset(total_blocks);

  // after accounting for the size of the bitset figure out how many blocks
  // can now be allocated
  size_t new_total_blocks =
      calculate_num_blocks(block_size, total_memory_available - bitset_size);

  // if the new total number of blocks is the same as the old we have converged
  while (new_total_blocks != total_blocks) {
    new_total_blocks =
        calculate_num_blocks(block_size, total_memory_available - bitset_size);

    // getting the new
    bitset_size = size_of_bitset(new_total_blocks);
    total_blocks = new_total_blocks;
  }

  return total_blocks;
}

static void init_bin(Bin *bin, size_t bin_size, MmapAllocation allocation) {
  // setting the type of allocation
  bin->header = (AllocationHeader){.allocation_type = BIN_ALLOCATION_TYPE};

  // setting the mmap allocation
  bin->mmap_allocation = allocation;

  // setting the size of the bin
  bin->bin_size = bin_size;
  bin->prev = NULL;
  bin->next = NULL;

  // calculating the size of the bitset
  size_t num_bits = calculate_bitset_size(bin_size);

  // initializing bitset
  // since the bitset has to be at the end this is a clever way to get its start
  init_bitset(((BitSet *)(bin + 1)) - 1, num_bits);

  // Setting where the initial memory used for allocation begins
  bin->ptr = (void *)((char *)bin + sizeof(Bin) + size_of_bitset(num_bits) -
                      sizeof(BitSet));
}

// Allocates memory to the passed in bin
static void *allocate_mem_to_bin(Bin *bin) {
  // finding first free available slot
  ssize_t index = find_first_unmarked_bit(&bin->bitset);

  // if not index was found return null
  if (index == -1) {
    return NULL;
  }

  // mark block as used
  mark_bit(&bin->bitset, index);

  // return corresponding block of memory
  return (void *)((char *)bin->ptr + index * bin->bin_size);
}

void *bin_alloc(size_t size) {
  // First figure out what in to use
  // aka what bin size to use
  size_t index = bin_index(size);

  // The head of bins for the specific size
  Bin *head = bins[index];

  // finding the first free bin
  Bin *current = head;

  while (current) {
    // if there is free space in the bin allocate memory
    void *ptr = allocate_mem_to_bin(current);
    if (ptr) {
      return ptr;
    }

    // otherwise go to next bin
    current = current->next;
  }

  // if current is null we are out of memory and a new bin needs to be allocated
  // one page is always allocated
  MmapAllocation allocation = retrieve_page();

  // if the allocation fails return NULL
  if (allocation.ptr == NULL) {
    return NULL;
  }

  // initializing the bin
  Bin *bin = (Bin *)allocation.ptr;
  size_t bin_size = calculate_bin_size(index);
  init_bin(bin, bin_size, allocation);

  // setting bin as head of manager
  bin->prev = NULL;
  bin->next = head;
  if (head != NULL) {
    head->prev = bin;
  }
  bins[index] = bin;

  // allocating memory to bin
  void *ptr = allocate_mem_to_bin(bin);

  return ptr;
}

// Takes a pointer to memory to free as well as the bin which it belongs to
void bin_free(void *ptr, Bin *bin) {
  // now the index of the allocation needs to be derived
  // allocation is done according to this formula
  // start_ptr + index * bin_size = allocation_ptr
  // where start_ptr is the begining of the region of memory
  // hence the calculation to get the index is
  // (allocation_ptr - start_ptr) / bin_size = index
  size_t index = ((char *)ptr - (char *)bin->ptr) / bin->bin_size;

  // now that we have the index we can mark the bit as free in the bitlist
  unmark_bit(&bin->bitset, index);

  // if no memory is still allocated return the memory
  if (is_bin_empty(bin)) {
    size_t index = bin_index(bin->bin_size);

    // if prev is null than this bin is the head
    if (bin->prev == NULL) {
      bins[index] = bin->next;
      if (bin->next != NULL) {
        bin->next->prev = NULL;
      }
    } else {
      bin->prev->next = bin->next;

      // if there is a next
      if (bin->next != NULL) {
        bin->next->prev = bin->prev;
      }
    }

    // return the page to the store
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
