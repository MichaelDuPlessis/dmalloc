#import "../lib.typ": code_block

= Development of a Small Object Memory Allocator

== Introduction

Below the implementation of the small memory allocator is described. The allocator makes use of three other separate allocators each specialized for different tasks. The three allocators are as follows:
1. A bin allocator for small objects.
2. A free list allocator for medium objects.
3. A page allocator for large objects.
The memory allocator makes exclusive use of the `mmap` system call to allocate a page of memory at a time. The allocator works by allocating a page of memory and assigning it a tag so that the kind of allocator used to allocate the memory is known. The three allocators will never share a page of memory and each sub-allocator will request new pages of memory as needed. All metadata for the memory allocated is stored in the page which it is allocated, this includes the tag specifying the kind of allocator used to allocate the memory as well as any associated metadata required. This means that when more memory is required the allocator will make a syscall to get a new memory page and therefore the smallest unit of measurement on the allocator level is a page of memory. The exact size of a page is not stated in this document as it is operating system specific. The sub-memory allocators are also lazy so that they only request memory from a page when it is needed and no memory is preallocated.

The general process for allocating memory is as follows:
1. An amount of memory is requested.
2. The root allocator determines what sub-allocator to use to allocate the memory.
3. The root allocator calls the appropriate sub-allocator passing along the amount of memory requested as well as any other required parameters.
4. The sub-allocator allocates the memory and returns a pointer to the memory.

The general process for deallocating memory is as follows:
1. A pointer to memory to be freed is passed to the root allocator:
2. The root allocator determines the page start for the page which the pointer belongs in. This is possible to calculate since `mmap` guarantees to return page aligned memory blocks.
3. The root allocator determines which sub-allocator allocated the memory from the tag embedded in the page start of the page of memory.
4. The root allocator calls the appropriate sub-allocator passing along the amount of memory requested as well as any other required parameters.
5. The memory is freed by the sub-allocator.

#figure(
  caption: [The general structure of how memory is laid out for every allocator per memory page.],
  placement: none,
  image("../images/page_header.drawio.png"),
)

== Page Cache

Allocating and deallocating memory pages is a slow operation as it requires a system call. To mitigate this issue the bin and free list allocators receive their memory pages from a page store which acts like a cache of memory pages. When a memory page is requested the page store will try to find a free page in its cache, if no pages are available pages are allocated using `mmap` to fill up the cache with an extra page being allocated which is then returned to the allocator requesting the memory. When an allocator returns memory to the page store the store will keep hold of the page so that it can be given to another allocator at a later time if the buffer holding the free pages is not full. If the buffer is full the page is returned to the operating system. The default number of pages that the store keeps cached is 4 but this can be modified using at compile time using a the macro `STORE_SIZE`. The memory allocator that makes use of the page cache is responsible for assigning its metadata to the page returned. The page cache is just a drop in replacement for the `mmap` system call. When the program starts the page cache is initially empty.

#figure(
  caption: [How the different allocators are connected to one another.],
  image("../images/allocation_flow.drawio.png"),
)

#figure(
  caption: [How the bin and free list allocators request and return memory.],
  image("../images/allocator_process_flow.drawio.png"),
)

It may appear that the allocator leaks memory since it is never stated what it does with the cached memory pages when the program terminates but this is dealt with by the operating system as the OS will automatically clean up any memory pages allocated to the program on program termination.

=== Example

In this example the allocators are called in the following order:
1. The bin allocator is used to allocate 1 object.
2. The free list allocator is used to allocate 3 objects.
3. The The bin allocator allocates all its memory.
4. The bin allocator allocates 1 object.
5. The page allocator allocates an object.
6. The free list allocator deallocates all its objects.

For simplicity sake it is assumed all allocations to the bin allocator are of the same size class but each size class does use its own memory page and a memory page is not shared between size classes. It is also assumed in this scenario that the page cache has capacity to store 4 pages.

Based on the allocations that occur above this is what happens to the page cache:
1. The program starts, the page cache is empty.
2. The bin allocator tries to allocate 1 object (1).
  1. The bin allocator has not allocated pages so it requests a page from the page cache.
  2. Since the cache is empty $4 + 1$ pages are allocated.
  3. The $(5)$th page is given the the bin allocator.
  4. The bin allocator allocates its one object.
3. The free list allocator tries to allocate 3 objects (2).
  1. The free list allocator has not allocated pages so it requests a page from the page cache.
  2. The page cache has $4$ pages available so it returns on and now has $3$ pages available.
  3. The free list allocator allocates 3 the objects.
4. The bin allocator allocates whats left of its memory (3).
  1. Since the bin allocator has already received a page it allocates memory until it is full.
5. The bin allocator tries to allocate 1 object (4).
  1. The bin allocators memory page is full so it requests another page from the page cache.
  2. The page cache has $3$ pages available so it returns on and now has $2$ pages available.
  3. The bin allocator allocates the object.
6. The page allocator allocates an object (5).
  1. The page allocator does not make use of the page cache and directly uses `mmap` to get a memory page.
  2. The page allocator allocates the object.
7. The free list allocator deallocates all its objects (6).
  1. Since all the objects are deallocated the free list returns the page to the page cache.
  2. The page cache has $2$ pages out of its capacity of $4$ so the page from the free list allocator is added back the cache and the page cache now has $3$ memory pages cached.

#figure(
  caption: [A diagram illustrating how the page cache works.],
  image("../images/example_page_cache.drawio.png"),
) <example_page_cache>

The green blocks in the diagram represent memory pages that that the page cache has cached while the red represent an empty cell. Diagram @fig:example_page_cache has the steps with the corresponding actions that are taken.

== Bin Allocator

The bin allocator is used to store small objects. Small objects for my implementation of an allocator is anything less than or equal to 8 bytes. The allocator works by dividing a region of memory into fixed sized blocks all of the same size. There is a bin for each possible block size so that different block sizes don't exist in the same bin. Since memory alignment is very important to computers it does not make sense to have bins with a size that are not a power of two otherwise the memory will become unaligned or padding to reach alignment will need to be inserted. This means have non standard bins sizes (sizes that are not powers of 2) will either lead to degraded performance or guaranteed fragmentation on every allocation. For this reason bins are always a power of two and this means that the allocator has 4 bins:
1. 1 byte bin.
2. 2 byte bin.
3. 4 byte bin.
4. 8 byte bin.
Following the decision decision that all sub-allocators use a page as their smallest unit of measurement it also means that each bin will be at least one page. If more memory is needed (the bin has been filled) a new page is allocated and the pages are linked together using a linked list.

Searching for a free block in a bin can be very inefficient as depending on the page size and block size there can be thousands of free blocks that need to be search through. A common approach to speed this process up is to make use of a bitmask to mark blocks as free or in use.

=== Bitmask

A bitmask is a piece of memory (usually represented as a number data type) in memory where the each bit acts as a flag to represent something. This may sound similar to an array of booleans but it has a number of advantages: a 1 byte number can store 8 flags which would take at least 8 bytes if it was a boolean array since booleans are represented using 1 byte each. Further it is easy to check if all flags in a bitmask are marked or unmarked as they can be compared to the value 0 or the largest possible value for an unsigned $N$ byte number. A bitset has the following properties:
- Time to find first free bit: $O(N)$
- Time to mark a bit: $O(1)$
- Time to clear bitset: $(1)$

The issue with bitmasks is that modern day computes and processors typically work on 8 byte numbers and in some cases have the ability to use 16 byte numbers. This means that in the common case a maximum of 64 flags can be used and 128 flags in the 16 byte case. Typically the page size on Linux is 4 kilobytes and 16 kilobytes on MacOS which will result in thousands of blocks which is much more than a typical bitmask can be used to represent.

=== Bitset

A bitset is an extension of a bitmask that allows for $K$ flags to set. A bitmask works by containing multiple bitmasks of size $N$ typically in an array. Naturally a bitset cannot be as fast as a bitmask since it has to check many more bitmasks to possible find a free bit. The time complexity of a bitset with $M$ bitmasks is as follows:
- Time to find first free bit: $O(M N)$
- Time to mark a bit: $O(1)$
- Time to clear bitmask: $(M)$
Marking a bit takes the same amount of time because if a the $n$th bit needs to be marked the bitmask and bit in the bitmask can be easily derived since the size of the bitset is known. There are some optimisations that can be made it improve the speed of searching through the bitset to find the first free bit which is the main use case of the allocator. The first easy optimisation to do is to keep track of the last bit or bitmask with a free bit. This when when trying to find a free bit the whole bitset does not need to be searched every time. A typical manner to implement this is that when a bit is unmarked set the index to that bitmasks index. The other optimisation which is not always available is to use builtin functions such as `__builtin_ctzl` which counts trailing zeroes of a number and it will usually compile down to a single instruction if the processor supports it. This is unfortunately not available on all processors and compilers but if it is available it is much faster than checking each bit individually of a bitmask. This means the new time complexity for finding the first free bit is $O(M)$ since finding the first free bit in a bitmask is constant.

I make use of these optimisations in the bitset to ensure fast lookup time.

=== Implementation

The bin allocator works using a bitset as described above to keep track of what blocks are free so that allocation is fast. When a block is free the bit is unmarked so that future allocations can use that block. Further another optimisation that was implemented is that a last free bit index is not used in the same way as described above. Instead this index is only updated if the free block of memory is at a lower address than what this index points to. This means that the memory allocator guarantees that forever where that index is there will only be free blocks of memory at a higher memory address and never lower. Let the index be $I$ and have it bounded as such $0 <= I < M$ then the new time complexity for finding the first free bit is $O(M - I)$.

Each bin size consists of multiple bins connected through a linked list. When allocating the bins are search consecutively until a bin with a free block is found. If no free block is found a new bin is created, the memory allocated and then made the head of the linked list to ensure that next time an allocation happens it does not search through all the bins and has the highest chance of finding a free bin early.

When a bin is allocated the size of the bitset needs to be determined based on how much space is available after the metadata for the bin has been stored but this leads to an interesting problem: the bitset is stored before the available memory to allocate and is part of the bin's metadata but the larger the bitset the less blocks the bin can have but the less blocks the bin has the smaller the bitset and this cycle continues forever. The following algorithm is used to determine the size of the bitset:

#code_block(
  "Algorithm used to determine the size of the bitset."
)[```c
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

```]

This calculation is not fast but only needs to be run once per bitset creation.

==== Allocation

Allocation works by first determining what the linked list of bins that fits the size class of the amount of memory that is going to be allocated. The linked list is than searched until a bin with a free spot in memory is found. If no bin has any free memory a new bin is allocated and made the head of the linked list and the memory is allocated. The function `find_first_unmarked_bit` is used to find the first free index in a bin, if the function returns -1 then there is no free memory left in the bin. Once a free spot is found the corresponding block of memory is calculated by `index * size class`, the bit is marked to indicate the memory is in use and the ptr to the block of memory is returned.

==== Deallocation

Deallocation works by simply deriving the index of the bit that indicates the memory is in use and un-marking it. If after a free the bin is empty (no more memory is allocated) the bin is returned to the page store. If all bits in the bitset are un-marked then no memory is allocated through the bin.

#code_block(
  "The metadata stored along with every bin."
)[```c
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

```]

#figure(
  caption: [How the bins are connected.],
  image("../images/sub_allocator_bin.drawio.png"),
)

As mentioned already bins are linked together through a linked list. Note all bins in a linked list have blocks of the same size. There is a separate linked list for each size class of bin.

#figure(
  caption: [What a bin looks like internally.],
  image("../images/sub_allocator_bin_blocks.drawio.png"),
)

The red blocks are allocated memory while the purple blocks are free memory. The index pointing to the next free block will never point to a block that has free blocks of memory behind it.

Another important key is that there is no external fragmentation with the implementation of the memory allocator. This is because each bin only stores objects of a certain size and the placement blocks are pre-defined. There is however the possibility of internal fragmentation. For example if 3 bytes of memory is requested then it is allocated to the 4 byte bin since it is the smallest bin that it can fit into. This internal fragmentation is not ideal but is also impossible to avoid. Event if there was a 3 byte bin due to how memory alignment works and its requirements an extra byte of padding would need to be added anyway. In other words there is no loss by not having a 3 byte bin and just using the 4 byte bin. This also applies for other size which are not powers of 2.

== Free List Allocator

The free list allocator is used for anything that is larger than 8 bytes but smaller than half of a page size. The implementation is a basic free list implementation as described above and it uses a first bit algorithm. All memory allocated is also aligned to 8 bytes. The free list allocator follows the same structure as the bin allocator where it is a linked list of free lists where each free list corresponds to a memory page. At the beginning of each free list page metadata is stored such as what allocator is and any other metadata regarding the free list.

=== Implementation

The free list contains two other constructs within it:
1. `Blocks` which are blocks of free memory that can be split and allocated. These make up the actual free list.
2. `AllocHeaders` which are metadata about an actual allocation and are stored before the pointer returned to the user.

==== Allocation

The free list allocator allocates memory using a first fit algorithm. When memory is requested from the allocator it iterates over all the free lists and within each free list to find the first free block of memory that can fit the requested amount and the header information for the requested block. Once a suitable free block of memory is found it splits the block if the block is large enough to to accommodate another future allocation. For example if splitting the block results in a 1 byte block then it is not worth splitting the block since no memory can be allocated there. If all the free lists have been searched through and there was not a sufficiently large block a new memory page is requested using `mmap` and a free list is created, the memory allocated and the free list is then made the head of the free list linked list.

==== Deallocation

Deallocation works by taking the pointer address of the memory that needs to be deallocated and then calculating the address of the page start of the memory page that the address belongs to. Once we have the page start the the necessary metadata can be derived such as the head of the free list. Next the free list block before and after the address to be deallocated is found if a before and after free block exists since the address may be at the beginning or end of the valid memory range. Once this is done the metadata of the allocated block is retrieved since it just lies before the pointer pointing to the memory to be deallocated. Now we have three components: the first free memory block before the pointer to be deallocated, the first free memory block after the pointer to be deallocated and the size of the allocation. Using this information one of 4 things can happen:
1. A free block is inserted between the previous and next free block with all pointers updated. If there is not a before or after only the relevant blocks are update and the new free block is inserted before or after.
2. The previous free block is continuos to the memory to be freed and the blocks are coalesced into a bigger block.
3. The next free block is continuos to the memory to be freed and the blocks are coalesced into a bigger block
4. Both the previous and the next free block is continuos to the memory to be freed and the blocks are coalesced into a bigger block

If after a free the free list is empty (it has allocated not memory) the page is returned to the page store. Determining if the free list is free is done by checking if the head block of the free list's size is all the available memory in the free list.

#code_block(
  "The metadata stored along with every free list."
)[```c
// A chunk of memory that can be further subdivided into blocks
typedef struct Chunk {
  // Meta data about the kind of allocation
  AllocationHeader header;
  // The original allocation
  MmapAllocation mmap_allocation;
  // The previous chunk of memory
  struct Chunk *prev;
  // The next chunk of memory
  struct Chunk *next;
  // Where the first free block of memory is
  Block *block_head;
} Chunk;
```]

#code_block(
  "The metadata stored along with every block."
)[```c
// A block of memory
typedef struct Block {
  // The amount of memory available that can be allocated
  // this includes the memory that the block occupies
  size_t size;
  // Where the next free block is located
  struct Block *next;
} Block;
```]

As mentioned already bins are linked together through a linked list.

#code_block(
  "The metadata stored along with every AllocHeader."
)[```c
// A header for a block of allocated memory
typedef struct {
  // The amount of memory allocated for a block
  // including the header
  size_t size;
} AllocHeader;
```]

As seen free blocks are connected through a linked list with the head of the list stored in the metadata of the free list.

#figure(
  caption: [How the free lists are connected.],
  image("../images/sub_allocator_freelist.drawio.png"),
)

#figure(
  caption: [What a free list looks like internally.],
  image("../images/sub_allocator_freelist_blocks.drawio.png"),
)

== Page Allocator

The page allocator is used for any memory allocated that is larger than a page size. It is the simplest allocator and is used for very large objects or for preallocating large amounts of memory. It follows the same structure as the previous two allocators where the first item in its memory page is the kind of allocation followed by metadata of the allocation. The page allocator allocates directly allocates memory pages using the `mmap` system call.

=== Implementation

It will allocates the amount of pages necessary for the requested amount of memory. There is exactly one memory allocation for each huge allocation. This means that if two allocations of sufficient size are requested at least two memory pages are used (depending on the size of allocation) per allocation. This means that allocation is very fast since no region of memory needs to be searched but it is slow however in the since a system call needs to be made each time an allocation occurs. At the same time freeing memory is also fast since no searching has to occur but does also require a system call. Freeing memory is done by extracting the header information and than calling `unmmap` to release the memory back to the operating system.
#figure(
  caption: [How the free lists are connected.],
  image("../images/sub_allocator_page.drawio.png"),
)

Unlike the other allocators page allocations are not linked to one another in any way.

== Allocator with Malloc

An alternative implementation of the memory allocator is possible where a combination of the bin allocator and the default malloc allocator is used. In this scenario the small object memory allocator is used as it previously was but when a memory allocation which exceeds the size class of the bin allocator is requested the default malloc implementation is used instead. In other words malloc takes the place of the free list and page allocator. This is useful to do if one if only specialising in some types of allocations.

This way of allocating memory was also tested and will be used when benchmarking and comparing.

== Example

Assuming a page size of 4kb and the bin allocator has 4 bins, 1b, 2b, 4b, 8b and using the following allocations and deallocations as an example:

#table(
  columns: 4,
  table.header([step], [allocation size], [amount], [allocator used]),
  [1], [3kb], [1], [page],
  [2], [247b], [3], [free list],
  [3], [4b], [10], [bin (4b)],
  [4], [1b], [100], [bin (1b)],
  [5], [1kb], [1], [free list],
  [6], [3b], [7], [bin (2b)],
  [7], [8b], [1], [bin (4b)],
  [8], [1b], [4096], [bin (1b)],
),

The following is what would happen in a running program:

- The page allocator makes a system call to `mmap` allocators the memory and returns a pointer to it.
- The free list allocator requests a page from the page cache allocators the first object and then subsequently allocates the second two. Since this is the first time the page cache is used this is when pages are populated in the cache.
- The bin allocator for bin size 4b requests a page from the page cache and then one by one allocates the 10 objects.
- The bin allocator for bin size 1b requests a page from the page cache and then one by one allocates the 100 objects.
- Since the allocation amount is less than half of the page size the free list allocator allocates the memory in the same page as its previous allocation.
- The bin allocator allocates seven 3 byte objects in the same bin as the 4 byte objects since it is the smallest bin that can accommodate 3b. No new memory pages are requested.
- The bin allocator for bin size 8b requests a page from the page cache and then allocates the memory.
- Many objects are allocated using the 1b bin allocator so it must request another page from the page cache.

All the allocators mentioned when requesting memory either using the page cache or `mmap` directly will themselves insert whatever metadata they need to the beginning of the page.

To further this example use the same allocations as before but have the following deallocations occur:
- Deallocate the 3 objects allocated to the free list in step 2 between steps 3 and 4.
- After step 6 dealloacte the allocation made in step one.

Now after step 3 all the memory allocated to the free list allocator is deallocate bu more importantly the memory page where the objects were allocated too no longer has any memory allocated to it so when the the last 247b object is deallocated the page is returned to the page cache and since the page cache won't be full because in the previous step the bin allocator took a page the page is stored for later use. When the free list allocator is used again to allocate the object in step 5 a new memory page will be requested from the page cache. When deallocating the object allocated in step 1 a simple call to `munmap` is made since the page allocator does not interface with the page cache.

== Optimisations

These are some optimisations that were made that are not memory allocator specific but still help with performance:

=== Using pointer arithmatic over array indexing

Since there is not repeated arithmetic to derive the value's address this can lead to significant speedups as there a pointer addition is a single operation whereas array indexing
often requires a multiplication and an addition.

#code_block(
  "Using indexing to traverse an array."
)[```c
  int x[100] = get_array();

  for (int i = 0; i < 100; i++) {
    do_something(x[i]);
  }
```]

#code_block(
  "Using pointer arithmetic to traverse an array."
)[```c
  int x[100] = get_array();

  int *y = x;
  for (int i = 0; i < 100; i++) {
    do_something(*y);
    y += 1;
  }
```]

=== Prefetching

Using prefetching to fetch data that will be needed before it is required can reduce cache misses thus increasing speadup. Prefetching is achieved using the compiler
intrinsic `__builtin_prefetch`.

=== Branch Prediction

Modern CPU's make use of pipelining to fetch multiple instructions in parallel. For example while the cpu is executing the current instruction it can fetch the next one
but `if` statements in programming present a problem as it is not known what branch of the `if` statement will execute at runtime and while modern processors do have
sophisticated branch prediction algorithms they can be wrong in which case a cache miss occurs and the data has too be fetched a second time.

Using the compiler builtin `__builtin_expect` it is possible to tell the CPU what branch it can expect to execute next. This is not a silver bullet however as this only leads
to performance increases if one branch of the `if` statement is selected disproportionately to the other, in scenarios where each branch is evaluated more or less the same
amount using `__builtin_expect` can degrade performance. Branch prediction is used in the memory allocator in places where there are outlier cases, for example:

#code_block("An example of where branch prediction is used.")[
  ```c

void mark_bit(BitSet *bitset, size_t index) {
  // Fast bounds check with branch prediction hint
  if (__builtin_expect(index >= bitset->num_bits, 0))
    return;

  /*
    Code
  */
}
  ```
]

In the scenario above most of the time the index provided for the bit to be marked will be a valid index so it is expected to evaluate to false.

=== Loop Unrolling

Loops are not free they require setup. For example a classic for loop in assembly requires a register to hold the count as well as an increment operation and
then also a comparision operation. If this loop is run from the beginning frequently the costs add up. Now if the loop contains an early return such as a
`break`, `goto` or `return` statement and if this early return  is triggered frequently on the first iteration of the loop it can be a performance gain to
first evaluate what would be the first element in the loop and only if that is false run the loop.

#code_block("An example of a for loop which has not been unrolled.")[
  ```c
  bool exists_in_array(int *arr, size_t size, int num) {
    for (int i = 0; i < size; i++) {
      if (arr[i] == num) {
        return true;
      }
    }

    return false
  }
  ```
]

#code_block("An example of a for loop which has been unrolled.")[
  ```c
  bool exists_in_array(int *arr, size_t size, int num) {
    if (size > 0 && arr[0] == num) {
      return true;
    }

    for (int i = 1; i < size; i++) {
      if (arr[i] == num) {
        return true;
      }
    }

    return false
  }
  ```
]

Assuming for this workload we know that for `num` we are checking it frequently appears as the first element in the array then unrolling the loop can speed up the code. This
can then further be extended by checkin the 2nd, 3rd, .., nth element. This optimisation only works if you understand your workload and shouldn't be blindly used as it bloats
the final binary and larger binaries are also bad for program speed.

The implementation of the memory allocator uses this technique in the `find_first_unmarked_bit` function as frequently the first word checked will have an unmarked bit.

== Limitations

The implementation of the allocator has the following limitations:
- It only works on UNIX based operating systems.
- It currently does not support multithreaded environments.

== Conclusion

Overall the implementation of the allocator consists of three sub allocator implementations each for a different size class, small, medium and large. The allocator is also completely page oriented only relying on the `mmap` system call for allocating memory. The allocator also has makes use of a page cache to avoid unnecessary system calls.
