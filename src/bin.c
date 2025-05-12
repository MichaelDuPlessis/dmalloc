#include "bin.h"
#include "allocator.h"
#include "bitset.h"
#include "mmap_allocator.h"
#include <stdio.h>

// Calculates the number of blocks that can fit in some amount of memory
size_t calculate_num_blocks(size_t block_size, size_t total_memory) {
  return total_memory / block_size;
}

// Calculates the number of bits needed for the bitset
size_t calculate_bitset_size(size_t block_size) {
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

void init_bin(Bin *bin, size_t bin_size) {
  // setting the type of allocation
  bin->header = (AllocationHeader){.allocation_type = BIN_ALLOCATION_TYPE};

  // setting the size of the bin
  bin->bin_size = bin_size;
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

void *bin_alloc(Bin *bin) {
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

void bin_free(void *ptr) {
  // pages are aligned to their allocation size
  // this can be used to figure out where the region
  // of memory starts and thus get the header information

  // getting page start
  void *page_start = calculate_page_start(ptr);

  // now that we have the start of the page the header information
  // can be extracted
  Bin *bin = (Bin *)page_start;

  // now the index of the allocation needs to be derived
  // allocation is done according to this formula
  // start_ptr + index * bin_size = allocation_ptr
  // where start_ptr is the begining of the region of memory
  // hence the calculation to get the index is
  // (allocation_ptr - start_ptr) / bin_size = index
  size_t index = ((char *)ptr - (char *)bin->ptr) / bin->bin_size;

  // now that we have the index we can mark the bit as free in the bitlist
  unmark_bit(&bin->bitset, index);
}

void *bin_manager_alloc(BinManager *manager) {
  // finding the first free bin
  Bin *current = manager->head;

  while (current) {
    // if there is free space in the bin allocate memory
    void *ptr = bin_alloc(current);
    if (ptr) {
      return ptr;
    }

    // otherwise go to next bin
    current = current->next;
  }

  // if current is null we are out of memory and a new bin needs to be allocated
  // one page is always allocated
  MmapAllocation allocation = mmap_alloc(1);

  // if the allocation fails return NULL
  if (allocation.ptr == NULL) {
    return NULL;
  }

  Bin *bin = (Bin *)allocation.ptr;
  init_bin(bin, manager->bin_size);
  // setting bin as head of manager
  bin->next = manager->head;
  manager->head = bin;

  // allocating memory to bin
  void *ptr = bin_alloc(bin);
  return ptr;
}

void bin_manager_free(void *ptr) {
  // the manager does not know what bin this memory belongs to
  bin_free(ptr);
}

void bin_manager_free_all(BinManager *manager) {
  Bin *current = manager->head;
  while (current) {
    Bin *next = current->next;

    // deallocating memory
    MmapAllocation allocation = {
        .ptr = (void *)current,
        .size = 1,
    };
    mmap_free(allocation);

    current = next;
  }

  manager->head = NULL;
}
